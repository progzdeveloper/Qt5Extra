#pragma once
#include <QStringList>
#include <QVector>
#include <QtTextExtra>

namespace Qt5Extra
{
    enum class LangNameFormat
    {
        LangFormat_ISO639,
        LangFormat_BCP47,
        LangFormat_HumanReadable,
        LangFormat_Default = LangFormat_ISO639
    };

    QTTEXTEXTRA_EXPORT QChar::Script localeToScriptCode(const QString& locale) Q_DECL_NOTHROW;
    QTTEXTEXTRA_EXPORT QStringList systemLanguages(LangNameFormat format = LangNameFormat::LangFormat_Default);
    QTTEXTEXTRA_EXPORT QVector<QChar::Script> supportedScripts();
}
