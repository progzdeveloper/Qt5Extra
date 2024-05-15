#include "cld2langdetector.h"
#include <cld2/public/compact_lang_det.h>
#include <QLocale>
#include <QtTextUtils>
#include <QSet>

QStringList CDL2LangDetector::identify(const QStringRef &text) const
{
    constexpr int kMaxResults = 3;
    CLD2::Language lang[kMaxResults];
    int percents[3];
    bool is_reliable;
    int textBytes = 0;
    const QByteArray ba = text.toUtf8();
    CLD2::DetectLanguageSummary(ba.data(), ba.size(), true, lang, percents, &textBytes, &is_reliable);

    detected.clear();
    detected.reserve(kMaxResults);
    for (int i = 0; i < kMaxResults; ++i)
    {
        if (lang[i] == CLD2::UNKNOWN_LANGUAGE)
            continue;

        auto result = langCache.emplace(lang[i], QString{});
        if (result.second)
        {
            const char* code = CLD2::LanguageCode(lang[i]);
            result.first->second = QLocale{ QString::fromLatin1(code) }.name();
        }
        detected << result.first->second;
    }
    return detected;
}
