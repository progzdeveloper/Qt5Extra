#pragma once

#include <QChar>

class QStringView;

namespace SpellChecking
{
    QChar::Script wordScript(QStringView word) noexcept;
    QChar::Script localeToScriptCode(const QString &locale) noexcept;

    bool isWordSkippable(QStringView word);
}
