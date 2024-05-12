#pragma once
#include <QStringList>

#include <QtSpellChecking>

class QTSPELLCHECKING_EXPORT QtSpellCheckBackend
{
public:
    virtual ~QtSpellCheckBackend() = default;

    virtual bool load();
    virtual bool unload();
    virtual bool validate(const QString&) const;
    virtual QStringList suggestions(const QString&, int) const;
    virtual void append(const QString&);
    virtual void remove(const QString&);
    virtual void ignore(const QString&);
    virtual bool contains(const QString&) const;
    virtual QStringList supportedLanguages() const;
};

