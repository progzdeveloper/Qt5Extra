#pragma once
#include <QSet>
#include <QString>

#include <QtSpellChecking>

class QTSPELLCHECKING_EXPORT QtSpellCheckBackend
{
public:
    struct SpellingProvider
    {
        QString name;
        QString library;
        QString description;
    };

    virtual ~QtSpellCheckBackend() = default;

    virtual bool load();
    virtual bool unload();
    virtual bool validate(const QString&, const QStringList& = {}) const;
    virtual QStringList suggestions(const QString&, int, const QStringList& = {}) const;
    virtual void append(const QString&);
    virtual void remove(const QString&);
    virtual void ignore(const QString&);
    virtual bool contains(const QString&) const;
    virtual QStringList supportedLanguages() const;
    virtual QList<SpellingProvider> providers() const;
};

