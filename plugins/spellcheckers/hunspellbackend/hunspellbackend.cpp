#include "hunspellbackend.h"
#include <unordered_map>
#include <memory>

#include <hunspell/hunspell.hxx>

#include <QLatin1String>
#include <QString>
#include <QTextCodec>
#include <QRegularExpression>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QDebug>
#include <QCoreApplication>

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


class HunspellBackendPrivate
{
public:
    using UStringMap = std::unordered_map<QString, QString>;
    using HunspellUptr = std::unique_ptr<Hunspell>;

    struct Entry
    {
        QTextCodec* codec = nullptr;
        HunspellUptr checker;

        Entry() = default;
        Entry(QTextCodec* codec, HunspellUptr&& speller)
            : codec(codec)
            , checker(std::move(speller))
        {}
    };
    std::unordered_map<QString, Entry> spellcheckers;

    static QString detectEncoding(const QString& affixFilePath)
    {
        // detect encoding analyzing the SET option in the affix file
        QString encoding = QStringLiteral("ISO8859-15");
        QFile affixFile(affixFilePath);
        if (!affixFile.open(QIODevice::ReadOnly))
            return encoding;

        QTextStream stream(&affixFile);
        QRegularExpression encDetector(
                    QStringLiteral("^\\s*SET\\s+([A-Z0-9\\-]+)\\s*"),
                    QRegularExpression::CaseInsensitiveOption);
        QString line;
        QRegularExpressionMatch match;
        while (!stream.atEnd())
        {
            line = stream.readLine();
            if (line.isEmpty())
                continue;
            match = encDetector.match(line);
            if (!match.hasMatch())
                continue;

            encoding = match.captured(1);
            break;
        }
        return encoding;
    }

    static QStringList buildSubDirsList(const QString& path)
    {
        if (!QFileInfo::exists(path))
            return {};

        QStringList dirs = { path };

        const auto& entryList = QDir(path).entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (const QFileInfo& subDir : entryList)
            dirs.push_back(subDir.absoluteFilePath());

        return dirs;
    }

    static void detectInstalledLangs(const QStringList& searchDirs, UStringMap& paths, UStringMap& aliases)
    {
        for (const QString& dirPath : searchDirs)
        {
            const auto entryList = QDir(dirPath).entryInfoList({ QLatin1String("*.aff"), QLatin1String("*.dic") }, QDir::Files);
            for (const QFileInfo& dict : entryList)
            {
                const QString language = dict.baseName();
                if (!dict.isSymLink())
                {
                    paths.emplace(language, dict.canonicalPath());
                    continue;
                }

                const QString alias = QFileInfo(dict.canonicalFilePath()).baseName();
                if (language != alias)
                    aliases.emplace(language, alias);
            }
        }
    }

    void emplaceSpeller(const QString& language, const QString& dirPath)
    {
        if (language.isEmpty())
            return;

        const QString dictFilePath  = QDir(dirPath).absoluteFilePath(language + QLatin1String(".dic"));
        const QString affixFilePath = QDir(dirPath).absoluteFilePath(language + QLatin1String(".aff"));

        if (!QFileInfo::exists(dictFilePath) || !QFileInfo::exists(affixFilePath))
        {
            qWarning() << "Unable to load dictionary for" << language << "in path" << dirPath;
            return;
        }

        const QString encoding = detectEncoding(affixFilePath);
        auto codec = QTextCodec::codecForName(encoding.toLatin1());
        auto speller = std::make_unique<Hunspell>(affixFilePath.toLocal8Bit().constData(),
                                                  dictFilePath.toLocal8Bit().constData());

        Q_ASSERT(speller != nullptr);
        spellcheckers[language] = { codec, std::move(speller) };
    }

    void createSpellers(const UStringMap& languagePaths, const UStringMap& languageAliases)
    {
        spellcheckers.reserve(languagePaths.size());

        QString language;
        for (const auto& [lang, path] : languagePaths)
        {
            auto it = languageAliases.find(language);
            if (it != languageAliases.end())
                language = it->second;
            else
                language = lang;

            emplaceSpeller(language, path);
        }
    }
};


HunspellBackend::HunspellBackend()
    : d(new HunspellBackendPrivate)
{}

HunspellBackend::~HunspellBackend() = default;

bool HunspellBackend::load()
{
    QStringList searchDirs;

    // add QStandardPaths
    searchDirs.append(QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QLatin1String("hunspell"), QStandardPaths::LocateDirectory));
    // add app resources
    searchDirs.append(d->buildSubDirsList(qApp->applicationDirPath()));
    // search additional user paths
    searchDirs.append(d->buildSubDirsList(QLatin1String("/System/Library/Spelling")));
    searchDirs.append(d->buildSubDirsList(QLatin1String("/usr/share/hunspell/")));
    searchDirs.append(d->buildSubDirsList(QLatin1String("/usr/share/myspell/")));

    HunspellBackendPrivate::UStringMap languagePaths, languageAliases;
    d->detectInstalledLangs(searchDirs, languagePaths, languageAliases);
    d->createSpellers(languagePaths, languageAliases);
    return true;
}

bool HunspellBackend::unload()
{
    d->spellcheckers.clear(); // clear and release memory
    return true;
}

bool HunspellBackend::validate(const QString& word, const QStringList& langs) const
{
    if (langs.isEmpty())
        return true;

    for (const auto& tag : langs)
    {
        auto it = d->spellcheckers.find(tag);
        if (it == d->spellcheckers.end())
            continue;

        const auto& e = it->second;
        std::string w = e.codec->fromUnicode(word.data(), word.size()).toStdString();
#if LIBHUNSPELL_VERSION > 150
        if (!e.checker->spell(w))
            return false;
#else
        if (e.checker->spell(w.data()))
            return true;
#endif
    }
    return false;
}

QStringList HunspellBackend::suggestions(const QString& word, int count, const QStringList& langs) const
{
    QSet<QString> stringSet;
    stringSet.reserve(count);
    for (const auto& tag : langs)
    {
        auto it = d->spellcheckers.find(tag);
        if (it == d->spellcheckers.end())
            continue;

        const auto& e = it->second;
        std::string w = e.codec->fromUnicode(word.data(), word.size()).toStdString();
#if LIBHUNSPELL_VERSION > 150
        const auto results = e.checker->suggest(w);
        for (const auto& s : results)
        {
            if (s.size() > 0)
                stringSet << e.codec_->toUnicode(s.data(), s.size());
            if (stringSet.size() >= count)
                break;
        }
#else
        char** results = nullptr;
        int n = e.checker->suggest(&results, w.data());
        for (int i = 0; i < n; ++i)
        {
            const char* s = results[i];
            const int l = qstrnlen(s, 256);
            if (l > 0)
                stringSet << e.codec->toUnicode(s, l);
            if (stringSet.size() >= count)
                break;
        }
        e.checker->free_list(&results, n);
#endif
        if (stringSet.size() >= count)
            break;
    }
    return stringSet.toList();
}

void HunspellBackend::append(const QString& word)
{
    for (const auto& e : d->spellcheckers)
    {
#if LIBHUNSPELL_VERSION > 150
        e.checker->add(e.codec->fromUnicode(word.data(), word.size()).toStdString());
#else
        const QByteArray ba = e.second.codec->fromUnicode(word.data(), word.size());
        e.second.checker->add(ba.data());
#endif
    }
}

void HunspellBackend::remove(const QString& word)
{
    for (const auto& e : d->spellcheckers)
    {
#if LIBHUNSPELL_VERSION > 150
        e.checker->remove(e.second.codec->fromUnicode(word.data(), word.size()).toStdString());
#else
        const QByteArray ba = e.second.codec->fromUnicode(word.data(), word.size());
        e.second.checker->remove(ba.data());
#endif
    }
}

void HunspellBackend::ignore(const QString& _word)
{
    append(_word);
}

QStringList HunspellBackend::supportedLanguages() const
{
    QStringList languages;
    languages.reserve(d->spellcheckers.size());
    for (const auto& e : d->spellcheckers)
        languages << e.first;
    return languages;
}

QList<QtSpellCheckBackend::SpellingProvider> HunspellBackend::providers() const
{
    static QList<SpellingProvider> providerList{ { "Hunspell", "N/A", "Hunspell library" } };
    return providerList;
}

