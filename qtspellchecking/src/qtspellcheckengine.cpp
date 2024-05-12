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

        QString word_;
        int offset_ = -1;
        int type_ = None;
        int count_ = 0;

        SpellCheckEvent() = default;

        SpellCheckEvent(int _type, const QString& _word, int _offset = -1, int _count = 0)
            : word_(_word)
            , offset_(_offset)
            , type_(_type)
            , count_(_count)
        {}
    };

    class SpellEventQueue
    {
        struct Entry
        {
            QObject* object_;
            std::deque<SpellCheckEvent> queue_;

            explicit Entry(QObject* _obj) : object_(_obj) {}

            void emplace(int _type, const QString& _word, int _offset, int _count)
            {
                queue_.emplace_back(_type, _word, _offset, _count);
            }

            void pop(SpellCheckEvent& _event)
            {
                _event = std::move(queue_.front());
                queue_.pop_front();
            }

            bool empty() const { return queue_.empty(); }

            void clear() { queue_.clear(); }

            void swap(Entry& _other)
            {
                using std::swap; // for Koenig lookup
                swap(object_, _other.object_);
                swap(queue_, _other.queue_);
            }
        };

        auto findQueue(QObject* _object)
        {
            return std::find_if(queueMap_.begin(), queueMap_.end(), [_object](const auto& e) { return e.object_ == _object; });
        }

    public:
        ~SpellEventQueue()
        {
            QMutexLocker locker(&mutex_);
            valid_ = false;
        }

        void emplace(QObject* _object, int _type, const QString& _word, int _offset, int _count)
        {
            QMutexLocker locker(&mutex_);
            auto it = findQueue(_object);
            if (it == queueMap_.end())
                it = queueMap_.emplace(queueMap_.end(), _object);

            it->emplace(_type, _word, _offset, _count);
        }

        void discard(QObject* _object)
        {
            QMutexLocker locker(&mutex_);
            if (queueMap_.empty() || !valid_)
                return;

            auto it = findQueue(_object);
            if (it == queueMap_.end())
                return;

            if (queueMap_.size() != 1)
                *it = std::move(queueMap_.back());

            queueMap_.pop_back();
            current_ = 0;
        }

        void clear()
        {
            QMutexLocker locker(&mutex_);
            queueMap_.clear();
        }

        bool empty() const
        {
            QMutexLocker locker(&mutex_);
            return queueMap_.empty();
        }

        bool tryPop(SpellCheckEvent& _event, QObject*& _receiver)
        {
            QMutexLocker locker(&mutex_);
            if (queueMap_.empty() || !valid_)
                return false;

            auto it = queueMap_.begin() + current_;
            if (it->empty())
                return false;

            _receiver = it->object_;
            it->pop(_event);
            if (it->empty())
            {
                if (queueMap_.size() != 1)
                    it->swap(queueMap_.back());

                queueMap_.pop_back();
            }

            current_ = (!queueMap_.empty()) ? ((current_ + 1) % queueMap_.size()) : 0;
            return true;
        }

    private:
        mutable QMutex mutex_;
        std::vector<Entry> queueMap_;
        size_t current_ = 0;
        bool valid_ = true;
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
    QScopedPointer<QtSpellCheckBackend> backend_;
    QString preferredBackend, currentBackend;
    SpellEventQueue queue_;
    QMutex mtx_;
    QWaitCondition cv_;
    int suggestsCount_ = 1;
    bool ready_ = false;
    bool interrupted_ = false;

    explicit QtSpellCheckEnginePrivate(QtSpellCheckEngine* _engine)
        : q(_engine)
    {
        createBackend();
    }

    void createBackend()
    {
        auto& factoryInstance = QtSpellCheckBackendFactory::instance();
        if (!preferredBackend.isEmpty())
            backend_.reset(factoryInstance.createBackend(preferredBackend));
        if (!backend_)
            backend_.reset(factoryInstance.createBackend(factoryInstance.platformBackend()));
        if (!backend_)
            backend_.reset(new QtSpellCheckBackend);
    }

    static void updateCache(MisspelledCache& _cache, const QString& _word)
    {
        _cache.insert(_word);
        if (_cache.size() < kMaxCacheCapacity)
            return;

        auto it = _cache.cbegin();
        while (_cache.size() >= kMaxCacheCapacity) // make room for new entries
        {
            while (it != _cache.cend() && *it == _word)
                ++it;
            it = _cache.erase(it);
        }
    }

    void processSpellEvent(QObject* _object, const SpellCheckEvent& _event, MisspelledCache& _cache)
    {
        switch (_event.type_)
        {
        case SpellCheckEvent::SpellWord:
            spellEvent(_object, _event, _cache);
            break;
        case SpellCheckEvent::AppendWord:
            supplyEvent(_event, _cache);
            break;
        case SpellCheckEvent::RemoveWord:
            discardEvent(_event, _cache);
            break;
        case SpellCheckEvent::IgnoreWord:
            ignoreEvent(_event, _cache);
            break;
        case SpellCheckEvent::Suggestions:
            suggestionsEvent(_object, _event);
            break;
        default:
            break;
        }
    }

    void spellEvent(QObject* _object, const SpellCheckEvent& _event, MisspelledCache& _cache)
    {
        if (!backend_ || _event.word_.isEmpty() || _event.offset_ < 0) // poisoned message detected
        {
            Q_EMIT q->completed(_object);
            return;
        }

        if (_cache.contains(_event.word_))
        {
            Q_EMIT q->misspelled(_object, _event.word_, _event.offset_, true);
            return;
        }

        const bool isValid = backend_->validate(_event.word_);
        if (!isValid)
            updateCache(_cache, _event.word_);
        Q_EMIT q->misspelled(_object, _event.word_, _event.offset_, !isValid);
    }

    void supplyEvent(const SpellCheckEvent& _event, MisspelledCache& _cache)
    {
        _cache.remove(_event.word_);
        backend_->append(_event.word_);
        Q_EMIT q->appended(_event.word_);
    }

    void discardEvent(const SpellCheckEvent& _event, MisspelledCache& _cache)
    {
        updateCache(_cache, _event.word_);
        backend_->remove(_event.word_);
        Q_EMIT q->removed(_event.word_);
    }

    void ignoreEvent(const SpellCheckEvent& _event, MisspelledCache& _cache)
    {
        _cache.remove(_event.word_);
        backend_->ignore(_event.word_);
        Q_EMIT q->ignored(_event.word_);
    }

    void suggestionsEvent(QObject* _object, const SpellCheckEvent& _event)
    {
        QtSpellCheckEngine::CorrectionActions actions = QtSpellCheckEngine::NoActions;
        QStringList results;
        if (_event.word_.isEmpty())
            return;
        if (backend_->validate(_event.word_))
        {
            if (backend_->contains(_event.word_))
                actions |= QtSpellCheckEngine::RemoveWord;
        }
        else
        {
            actions |= (QtSpellCheckEngine::AppendWord | QtSpellCheckEngine::IgnoreWord);
            results = backend_->suggestions(_event.word_, _event.count_);
        }
        Q_EMIT q->suggestsFound(_object, _event.word_, results, actions);
    }

    void postSpellEvent(QObject* _receiver, int _type, const QString& _word, int _offset = -1, int _count = 0)
    {
        if (!q->isRunning())
            q->start();

        queue_.emplace(_receiver, _type, _word, _offset, _count);
        {
            QMutexLocker lk(&mtx_);
            ready_ = true;
        }
        cv_.notify_one();
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
        QMutexLocker lk(&d->mtx_);
        d->ready_ = true;
        d->interrupted_ = true;
    }

    d->cv_.notify_one();
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

void QtSpellCheckEngine::spell(const QString& _word, int _offset, QObject* _receiver)
{
    if (!d->backend_)
        return;

    d->postSpellEvent(_receiver, SpellCheckEvent::SpellWord, _word, _offset);
}

void QtSpellCheckEngine::append(const QString& _word)
{
    if (_word.isEmpty() || !d->backend_)
        return;

    d->postSpellEvent(nullptr, SpellCheckEvent::AppendWord, _word);
}

void QtSpellCheckEngine::remove(const QString& _word)
{
    if (_word.isEmpty() || !d->backend_)
        return;

    d->postSpellEvent(nullptr, SpellCheckEvent::RemoveWord, _word);
}

void QtSpellCheckEngine::ignore(const QString& _word)
{
    if (_word.isEmpty() || !d->backend_)
        return;

    d->postSpellEvent(nullptr, SpellCheckEvent::IgnoreWord, _word);
}

void QtSpellCheckEngine::requestSuggests(const QString& _word, int _count, QObject* _receiver)
{
    if (_word.isEmpty() || !d->backend_)
        return;

    d->postSpellEvent(_receiver, SpellCheckEvent::Suggestions, _word, -1, std::clamp(_count, d->kMinSuggests, d->kMaxSuggests));
}

void QtSpellCheckEngine::run()
{
    qRegisterMetaType<QtSpellCheckEngine::CorrectionActions>("SpellCheckEngine::CorrectionActions");

    d->ready_ = false;
    d->interrupted_ = false;

    if (d->backend_)
        d->backend_->load();

    QtSpellCheckEnginePrivate::MisspelledCache misspellingsCache;
    misspellingsCache.reserve(d->kMaxCacheCapacity);

    QObject* receiver = nullptr;
    SpellCheckEvent event;
    Q_FOREVER
    {
        if (d->queue_.tryPop(event, receiver))
        {
            d->processSpellEvent(receiver, event, misspellingsCache);
            continue;
        }

        QMutexLocker lk(&d->mtx_);
        while (!d->ready_)
            d->cv_.wait(&d->mtx_);
        d->ready_ = false;

        if (d->interrupted_)
        {
            lk.unlock();
            d->queue_.clear();
            break;
        }

        // Manual unlocking is done before notifying, to avoid waking up
        // the waiting thread only to block again (see notify_one for details)
        lk.unlock();
        d->cv_.notify_one();
    }

    if (d->backend_)
        d->backend_->unload();
}

void QtSpellCheckEngine::cancel(QObject* _object)
{
    {
        QMutexLocker lk(&d->mtx_);
        d->queue_.discard(_object);
        d->ready_ = true;
    }

    d->cv_.notify_one();
}

