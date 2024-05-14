#include "qtspellcheckbackend.h"

bool QtSpellCheckBackend::load()
{
    return true;
}

bool QtSpellCheckBackend::unload()
{
    return true;
}

bool QtSpellCheckBackend::validate(const QString&, const QStringList&) const
{
    return true;
}

QStringList QtSpellCheckBackend::suggestions(const QString&, int, const QStringList&) const
{
    return {};
}

void QtSpellCheckBackend::append(const QString&)
{
}

void QtSpellCheckBackend::remove(const QString&)
{
}

void QtSpellCheckBackend::ignore(const QString&)
{
}

bool QtSpellCheckBackend::contains(const QString&) const
{
    return false;
}

QStringList QtSpellCheckBackend::supportedLanguages() const
{
    return {};
}

QList<QtSpellCheckBackend::SpellingProvider> QtSpellCheckBackend::providers() const
{
    return {};
}

