#include "qtspellcheckengine.h"
#include "qtspellcheckbackend.h"
#include "qtspellcheckbackendfactory.h"
#include <QWaitCondition>
#include <QMutex>
#include <QString>
#include <QSet>
#include <deque>

namespace
{
    struct SpellCheckEvent
    {
        enum
        {
            None = -1,
            SpellWord = 0,
            AppendWord,
            RemoveWord,
            IgnoreWord,
            Suggestions
        };

        QString word;
        int offset = -1;
        int type = None;
        int count = 0;

        SpellCheckEvent() = default;

        SpellCheckEvent(int t, const QString& s, int i = -1, int n = 0)
            : word(s)
            , offset(i)
            , type(t)
            , count(n)
        {}
    };

    class SpellEventQueue
    {
        struct Entry
        {
            QObject* object;
            std::deque<SpellCheckEvent> queue;

            explicit Entry(QObject* obj) : object(obj) {}

            void emplace(int type, const QString& word, int offset, int count)
            {
                queue.emplace_back(type, word, offset, count);
            }

            void pop(SpellCheckEvent& event)
            {
                event = std::move(queue.front());
                queue.pop_front();
            }

            bool empty() const { return queue.empty(); }

            void clear() { queue.clear(); }

            void swap(Entry& other)
            {
                using std::swap; // for Koenig lookup
                swap(object, other.object);
                swap(queue, other.queue);
            }
        };

        auto findQueue(QObject* object)
        {
            return std::find_if(queueMap.begin(), queueMap.end(), [object](const auto& e) { return e.object == object; });
        }

    public:
        ~SpellEventQueue()
        {
            QMutexLocker locker(&mutex);
            valid = false;
        }

        void emplace(QObject* object, int type, const QString& word, int offset, int count)
        {
            QMutexLocker locker(&mutex);
            auto it = findQueue(object);
            if (it == queueMap.end())
                it = queueMap.emplace(queueMap.end(), object);

            it->emplace(type, word, offset, count);
        }

        void discard(QObject* object)
        {
            QMutexLocker locker(&mutex);
            if (queueMap.empty() || !valid)
                return;

            auto it = findQueue(object);
            if (it == queueMap.end())
                return;

            *it = std::move(queueMap.back());

            queueMap.pop_back();
            current = 0;
        }

        void clear()
        {
            QMutexLocker locker(&mutex);
            queueMap.clear();
        }

        bool empty() const
        {
            QMutexLocker locker(&mutex);
            return queueMap.empty();
        }

        bool tryPop(SpellCheckEvent& event, QObject*& receiver)
        {
            QMutexLocker locker(&mutex);
            if (queueMap.empty() || !valid)
                return false;

            auto it = queueMap.begin() + current;
            if (it->empty())
                return false;

            receiver = it->object;
            it->pop(event);
            if (it->empty())
            {
                it->swap(queueMap.back());
                queueMap.pop_back();
            }

            current = (!queueMap.empty()) ? ((current + 1) % queueMap.size()) : 0;
            return true;
        }

    private:
        mutable QMutex mutex;
        std::vector<Entry> queueMap;
        size_t current = 0;
        bool valid = true;
    };
}


class QtSpellCheckEnginePrivate
{
public:
    static constexpr size_t kMaxCacheCapacity = 1024;
    static constexpr int kMinSuggests = 1;
    static constexpr int kMaxSuggests = 10;

    using MisspelledCache = QSet<QString>;

    QtSpellCheckEngine* q;
    QScopedPointer<QtSpellCheckBackend> backend;
    QString preferredBackend, currentBackend;
    SpellEventQueue queue;
    QMutex mtx;
    QWaitCondition cv;
    bool ready = false;
    bool interrupted = false;

    explicit QtSpellCheckEnginePrivate(QtSpellCheckEngine* engine)
        : q(engine)
    {
        createBackend();
    }

    void createBackend()
    {
        auto& factoryInstance = QtSpellCheckBackendFactory::instance();
        if (!preferredBackend.isEmpty())
            backend.reset(factoryInstance.createBackend(preferredBackend));
        if (!backend)
            backend.reset(factoryInstance.createBackend(factoryInstance.platformBackend()));
        if (!backend)
            backend.reset(new QtSpellCheckBackend);
    }

    static void updateCache(MisspelledCache& cache, const QString& word)
    {
        cache.insert(word);
        if (cache.size() < kMaxCacheCapacity)
            return;

        auto it = cache.cbegin();
        while (cache.size() >= kMaxCacheCapacity) // make room for new entries
        {
            while (it != cache.cend() && *it == word)
                ++it;
            it = cache.erase(it);
        }
    }

    void processSpellEvent(QObject* object, const SpellCheckEvent& event, MisspelledCache& cache)
    {
        switch (event.type)
        {
        case SpellCheckEvent::SpellWord:
            spellEvent(object, event, cache);
            break;
        case SpellCheckEvent::AppendWord:
            supplyEvent(event, cache);
            break;
        case SpellCheckEvent::RemoveWord:
            discardEvent(event, cache);
            break;
        case SpellCheckEvent::IgnoreWord:
            ignoreEvent(event, cache);
            break;
        case SpellCheckEvent::Suggestions:
            suggestionsEvent(object, event);
            break;
        default:
            break;
        }
    }

    void spellEvent(QObject* object, const SpellCheckEvent& event, MisspelledCache& cache)
    {
        if (!backend || event.word.isEmpty() || event.offset < 0) // poisoned message detected
        {
            Q_EMIT q->completed(object);
            return;
        }

        if (cache.contains(event.word))
        {
            Q_EMIT q->misspelled(object, event.word, event.offset);
        }
        else if (!backend->validate(event.word))
        {
            updateCache(cache, event.word);
            Q_EMIT q->misspelled(object, event.word, event.offset);
        }
    }

    void supplyEvent(const SpellCheckEvent& event, MisspelledCache& cache)
    {
        cache.remove(event.word);
        backend->append(event.word);
        Q_EMIT q->appended(event.word);
    }

    void discardEvent(const SpellCheckEvent& event, MisspelledCache& cache)
    {
        updateCache(cache, event.word);
        backend->remove(event.word);
        Q_EMIT q->removed(event.word);
    }

    void ignoreEvent(const SpellCheckEvent& event, MisspelledCache& cache)
    {
        cache.remove(event.word);
        backend->ignore(event.word);
        Q_EMIT q->ignored(event.word);
    }

    void suggestionsEvent(QObject* object, const SpellCheckEvent& event)
    {
        QtSpellCheckEngine::SpellingActions actions = QtSpellCheckEngine::NoActions;
        QStringList results;
        if (event.word.isEmpty())
            return;

        if (backend->validate(event.word) &&
            backend->contains(event.word))
        {
            actions |= QtSpellCheckEngine::RemoveWord;
        }
        else
        {
            actions |= (QtSpellCheckEngine::AppendWord | QtSpellCheckEngine::IgnoreWord);
            results = backend->suggestions(event.word, event.count);
        }
        Q_EMIT q->suggestsFound(object, event.word, results, actions);
    }

    void postSpellEvent(QObject* receiver, int type, const QString& word, int offset = -1, int count = 0)
    {
        if (!q->isRunning())
            q->start();

        queue.emplace(receiver, type, word, offset, count);
        {
            QMutexLocker lk(&mtx);
            ready = true;
        }
        cv.notify_one();
    }
};


QtSpellCheckEngine::QtSpellCheckEngine()
    : QThread(nullptr)
    , d(new QtSpellCheckEnginePrivate(this))
{
}

QtSpellCheckEngine::~QtSpellCheckEngine()
{
    {
        QMutexLocker lk(&d->mtx);
        d->ready = true;
        d->interrupted = true;
    }

    d->cv.notify_one();
    wait();
    quit();
}

QtSpellCheckEngine& QtSpellCheckEngine::instance()
{
    static QtSpellCheckEngine gInstance;
    return gInstance;
}

void QtSpellCheckEngine::setPrefferedBackend(const QString& backend)
{
    d->preferredBackend = backend;
}

QString QtSpellCheckEngine::preferredBackend() const
{
    return d->preferredBackend;
}

QString QtSpellCheckEngine::backendName() const
{
    return d->currentBackend;
}

void QtSpellCheckEngine::spell(const QString& word, int offset, QObject* receiver)
{
    if (!d->backend)
        return;

    d->postSpellEvent(receiver, SpellCheckEvent::SpellWord, word, offset);
}

void QtSpellCheckEngine::append(const QString& word)
{
    if (word.isEmpty() || !d->backend)
        return;

    d->postSpellEvent(nullptr, SpellCheckEvent::AppendWord, word);
}

void QtSpellCheckEngine::remove(const QString& word)
{
    if (word.isEmpty() || !d->backend)
        return;

    d->postSpellEvent(nullptr, SpellCheckEvent::RemoveWord, word);
}

void QtSpellCheckEngine::ignore(const QString& word)
{
    if (word.isEmpty() || !d->backend)
        return;

    d->postSpellEvent(nullptr, SpellCheckEvent::IgnoreWord, word);
}

void QtSpellCheckEngine::requestSuggests(const QString& word, int count, QObject* receiver)
{
    if (word.isEmpty() || !d->backend)
        return;

    d->postSpellEvent(receiver, SpellCheckEvent::Suggestions, word, -1, std::clamp(count, d->kMinSuggests, d->kMaxSuggests));
}

void QtSpellCheckEngine::run()
{
    qRegisterMetaType<QtSpellCheckEngine::SpellingActions>("SpellCheckEngine::SpellingActions");

    d->ready = false;
    d->interrupted = false;

    if (d->backend)
        d->backend->load();

    QtSpellCheckEnginePrivate::MisspelledCache misspellingsCache;
    misspellingsCache.reserve(d->kMaxCacheCapacity);

    QObject* receiver = nullptr;
    SpellCheckEvent event;
    Q_FOREVER
    {
        if (d->queue.tryPop(event, receiver))
        {
            d->processSpellEvent(receiver, event, misspellingsCache);
            continue;
        }

        QMutexLocker lk(&d->mtx);
        while (!d->ready)
            d->cv.wait(&d->mtx);
        d->ready = false;

        if (d->interrupted)
        {
            lk.unlock();
            d->queue.clear();
            break;
        }

        // Manual unlocking is done before notifying, to avoid waking up
        // the waiting thread only to block again (see notify_one for details)
        lk.unlock();
        d->cv.notify_one();
    }

    if (d->backend)
        d->backend->unload();
}

void QtSpellCheckEngine::cancel(QObject* object)
{
    {
        QMutexLocker lk(&d->mtx);
        d->queue.discard(object);
        d->ready = true;
    }

    d->cv.notify_one();
}

