#include "enchantbackend.h"
#include <enchant/enchant.h>
#include <memory>
#include <QVarLengthArray>
#include <QSet>
#include <QDebug>

#if (QT_VERSION < QT_VERSION_CHECK(5, 14, 0))
namespace std
{
    template<>
    struct hash<QString>
    {
        inline std::size_t operator()(const QString& key) const
        { return qHash(key); }
    };
} // namespace std
#endif

class EnchantSpeller
{
    Q_DISABLE_COPY(EnchantSpeller)
public:
    EnchantSpeller(EnchantDict* d, EnchantBroker* b)
        : mDict(d)
        , mBroker(b)
    {
        enchant_dict_describe(mDict, load, this);
    }

    ~EnchantSpeller()
    {
        enchant_broker_free_dict(mBroker, mDict);
    }

    bool validate(const QByteArray& utf8Word) const
    {
        return (enchant_dict_check(mDict, utf8Word.data(), utf8Word.size()) == 0);
    }

    void suggest(const QByteArray& utf8Word, QSet<QString>& suggestions, size_t n) const
    {
        size_t n_suggs = 0;
        char** suggs = nullptr;

        suggs = enchant_dict_suggest (mDict, utf8Word.data(), utf8Word.size(), &n_suggs);

        if (suggs && n_suggs)
        {
            for (size_t i = 0, k = std::min(n, n_suggs); i < k; ++i)
                suggestions.insert(QString::fromUtf8(suggs[i]));

            enchant_dict_free_string_list (mDict, suggs);
        }
    }

    void append(const QByteArray& utf8Word)
    {
        enchant_dict_add(mDict, utf8Word.data(), utf8Word.size());
    }

    void remove(const QByteArray& utf8Word)
    {
        enchant_dict_remove(mDict, utf8Word.data(), utf8Word.size());
    }

    void ignore(const QByteArray& utf8Word)
    {
        enchant_dict_add_to_personal(mDict, utf8Word.data(), utf8Word.size());
    }

    QString language() const { return mLanguage; }
    QString providerName() const { return mProviderName; }
    QString providerPath() const { return mProviderPath; }
    QString providerComment() const { return mProviderComment; }

private:
    static void load(const char * const lang,
                     const char * const providerName,
                     const char * const providerDesc,
                     const char * const providerFile,
                     void * userData)
    {
        EnchantSpeller* dict = static_cast<EnchantSpeller*>(userData);
        if (!dict)
            return;

        qDebug() << "[EnchantSpeller] loading dictionary:" << lang << providerName << providerFile << providerDesc;
        dict->mLanguage = QString::fromUtf8(lang);
        dict->mProviderName = QString::fromUtf8(providerName);
        dict->mProviderPath = QString::fromUtf8(providerFile);
        dict->mProviderComment = QString::fromUtf8(providerDesc);
    }

private:
    QString mLanguage;
    QString mProviderName;
    QString mProviderPath;
    QString mProviderComment;
    EnchantDict* mDict;
    EnchantBroker* mBroker;
};

class EnchantDictBroker
{
    Q_DISABLE_COPY(EnchantDictBroker)
public:
    EnchantDictBroker()
    {
        qDebug() << "[EnchantDictBroker]: creating broker";
        broker = enchant_broker_init();
        if (!broker)
            qCritical() << "[EnchantDictBroker]: failed to create broker";
    }

    ~EnchantDictBroker()
    {
        qDebug() << "[EnchantDictBroker]: release broker";
        enchant_broker_free(broker);
    }

    operator EnchantBroker* () const { return broker; };

private:
    EnchantBroker* broker;
};


class EnchantBackendPrivate
{
public:
    using EnchantUptr = std::unique_ptr<EnchantSpeller>;

    QList<QtSpellCheckBackend::SpellingProvider> providers;
    std::unordered_map<QString, EnchantUptr> spellcheckers;
    EnchantDictBroker broker;

    void requestSpeller(const QByteArray& language)
    {
        EnchantDict* dict = enchant_broker_request_dict(broker, language.data());
        if (!dict)
            return;

        spellcheckers[language] = std::make_unique<EnchantSpeller>(dict, broker);
    }

    static void loadSpeller(const char * const lang,
                            const char * const providerName,
                            const char * const providerDesc,
                            const char * const providerFile,
                            void * userData)
    {
        using Provider = QtSpellCheckBackend::SpellingProvider;
        EnchantBackendPrivate* d = static_cast<EnchantBackendPrivate*>(userData);
        if (!d)
            return;

        qDebug() << "[EnchantBackend] loading speller:" << lang << providerName << providerFile << providerDesc;
        QByteArray ba;
        ba.setRawData(lang, qstrnlen(lang, 16));
        d->requestSpeller(ba);
        d->providers << Provider{ QString::fromUtf8(providerName), QString::fromUtf8(providerFile), QString::fromUtf8(providerDesc) };
    }
};


EnchantBackend::EnchantBackend()
    : d(new EnchantBackendPrivate)
{
}

EnchantBackend::~EnchantBackend() = default;

bool EnchantBackend::load()
{
    enchant_broker_list_dicts(d->broker, &EnchantBackendPrivate::loadSpeller, d.get());
    return true;
}

bool EnchantBackend::unload()
{
    d->providers.clear();
    d->spellcheckers.clear();
    return true;
}

bool EnchantBackend::validate(const QString &word, const QStringList& langs) const
{
    if (langs.isEmpty())
        return {};

    const QByteArray ba = word.toUtf8();
    for (const QString& tag : langs)
    {
        auto it = d->spellcheckers.find(tag);
        if (it == d->spellcheckers.end())
            continue;

        const auto& e = it->second;
        if (e->validate(ba.data()))
            return true;
    }
    return false;
}

QStringList EnchantBackend::suggestions(const QString &word, int count, const QStringList& langs) const
{
    QSet<QString> results;
    results.reserve(count);

    const QByteArray ba = word.toUtf8();
    for (const QString& tag : langs)
    {
        auto it = d->spellcheckers.find(tag);
        if (it == d->spellcheckers.end())
            continue;

        const auto& e = it->second;
        e->suggest(ba, results, count);
        if (results.size() >= count)
            break;
    }
    return results.toList();
}

void EnchantBackend::append(const QString &word)
{
    const QByteArray ba = word.toUtf8();
    for (const auto& e : d->spellcheckers)
        e.second->append(ba);
}

void EnchantBackend::remove(const QString &word)
{
    const QByteArray ba = word.toUtf8();
    for (const auto& e : d->spellcheckers)
        e.second->remove(ba);
}

void EnchantBackend::ignore(const QString &word)
{
    const QByteArray ba = word.toUtf8();
    for (const auto& e : d->spellcheckers)
        e.second->ignore(ba);
}

QStringList EnchantBackend::supportedLanguages() const
{
    QStringList languages;
    languages.reserve(d->spellcheckers.size());
    for (const auto& e : d->spellcheckers)
        languages << e.second->language();
    return languages;
}

QList<QtSpellCheckBackend::SpellingProvider> EnchantBackend::providers() const
{
    return d->providers;
}
