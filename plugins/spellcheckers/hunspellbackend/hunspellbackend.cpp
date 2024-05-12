//#ifdef HAS_HUNSPELL_LIBRARY
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

#include <QtGlobal>
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
        QString language_;
        QTextCodec* codec_ = nullptr;
        HunspellUptr checker_;

        Entry() = default;
        Entry(const QString& _lang, QTextCodec* _codec, HunspellUptr&& _speller)
            : language_(_lang)
            , codec_(_codec)
            , checker_(std::move(_speller))
        {}
    };
    QVarLengthArray<Entry, 16> spellcheckers_;

    static QString detectEncoding(const QString& _affixFilePath)
    {
        // detect encoding analyzing the SET option in the affix file
        QString encoding = QStringLiteral("ISO8859-15");
        QFile affixFile(_affixFilePath);
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

    static QStringList buildSubDirsList(const QString& _path)
    {
        if (!QFileInfo::exists(_path))
            return {};

        QStringList dirs = { _path };

        const auto& entryList = QDir(_path).entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (const QFileInfo& subDir : entryList)
            dirs.push_back(subDir.absoluteFilePath());

        return dirs;
    }

    static void detectInstalledLangs(const QStringList& _searchDirs, UStringMap& _paths, UStringMap& _aliases)
    {
        for (const QString& dirPath : _searchDirs)
        {
            const auto entryList = QDir(dirPath).entryInfoList({ QLatin1String("*.aff"), QLatin1String("*.dic") }, QDir::Files);
            for (const QFileInfo& dict : entryList)
            {
                const QString language = dict.baseName();
                if (!dict.isSymLink())
                {
                    _paths.emplace(language, dict.canonicalPath());
                    continue;
                }

                const QString alias = QFileInfo(dict.canonicalFilePath()).baseName();
                if (language != alias)
                    _aliases.emplace(language, alias);
            }
        }
    }

    void emplaceSpeller(const QString& _language, const QString& _dirPath)
    {
        if (_language.isEmpty())
            return;

        const QString dictFilePath  = QDir(_dirPath).absoluteFilePath(_language + QLatin1String(".dic"));
        const QString affixFilePath = QDir(_dirPath).absoluteFilePath(_language + QLatin1String(".aff"));

        if (!QFileInfo::exists(dictFilePath) || !QFileInfo::exists(affixFilePath))
        {
            qWarning() << "Unable to load dictionary for" << _language << "in path" << _dirPath;
            return;
        }

        qDebug() << LIBHUNSPELL_VERSION;

        const QString encoding = detectEncoding(affixFilePath);
        auto codec = QTextCodec::codecForName(encoding.toLatin1());
        auto speller = std::make_unique<Hunspell>(affixFilePath.toLocal8Bit().constData(),
                                                  dictFilePath.toLocal8Bit().constData());

        Q_ASSERT(speller != nullptr);
        spellcheckers_.push_back({ _language, codec, std::move(speller) });
    }

    void createSpellers(const UStringMap& _languagePaths, const UStringMap& _languageAliases)
    {
        spellcheckers_.reserve(_languagePaths.size());

        QString language;
        for (const auto& [lang, path] : _languagePaths)
        {
            auto it = _languageAliases.find(language);
            if (it != _languageAliases.end())
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
    d->spellcheckers_.clear();
    return true;
}

bool HunspellBackend::validate(const QString& _word) const
{
    for (const auto& e : d->spellcheckers_)
    {
        std::string w = e.codec_->fromUnicode(_word.data(), _word.size()).toStdString();
#if LIBHUNSPELL_VERSION > 150
        if (e.checker_->spell(w))
            return true;
#else
        if (e.checker_->spell(w.data()))
            return true;
#endif
    }
    return false;
}

QStringList HunspellBackend::suggestions(const QString& _word, int _count) const
{
    QSet<QString> stringSet;
    stringSet.reserve(_count);

    for (const auto& e : d->spellcheckers_)
    {
        std::string w = e.codec_->fromUnicode(_word.data(), _word.size()).toStdString();
#if LIBHUNSPELL_VERSION > 150
        const auto results = e.checker_->suggest(w);
        for (const auto& s : results)
        {
            if (s.size() > 0)
                stringSet << e.codec_->toUnicode(s.data(), s.size());
            if (stringSet.size() >= _count)
                break;
        }
#else
        char** results = nullptr;
        int n = e.checker_->suggest(&results, w.data());
        for (int i = 0; i < n; ++i)
        {
            const char* s = results[i];
            const int l = qstrnlen(s, 256);
            if (l > 0)
                stringSet << e.codec_->toUnicode(s, l);
            if (stringSet.size() >= _count)
                break;
        }
        e.checker_->free_list(&results, n);
#endif
    }
    return stringSet.toList();
}

void HunspellBackend::append(const QString& _word)
{
    for (const auto& e : d->spellcheckers_)
    {
#if LIBHUNSPELL_VERSION > 150
        e.checker_->add(e.codec_->fromUnicode(_word.data(), _word.size()).toStdString());
#else
        const QByteArray ba = e.codec_->fromUnicode(_word.data(), _word.size());
        e.checker_->add(ba.data());
#endif
    }
}

void HunspellBackend::remove(const QString& _word)
{
    for (const auto& e : d->spellcheckers_)
    {
#if LIBHUNSPELL_VERSION > 150
        e.checker_->remove(e.codec_->fromUnicode(_word.data(), _word.size()).toStdString());
#else
        const QByteArray ba = e.codec_->fromUnicode(_word.data(), _word.size());
        e.checker_->remove(ba.data());
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
    languages.reserve(d->spellcheckers_.size());
    for (const auto& e : d->spellcheckers_)
        languages << e.language_;
    return languages;
}

bool HunspellBackend::isAvailable()
{
    return true;
}
//#endif
