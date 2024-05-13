#pragma once
#include <QtSpellCheckBackend>
#include <QScopedPointer>

class HunspellBackend : public QtSpellCheckBackend
{
    // QtSpellCheckBackend interface
public:
    HunspellBackend();
    ~HunspellBackend() Q_DECL_OVERRIDE;
    bool load() Q_DECL_OVERRIDE;
    bool unload() Q_DECL_OVERRIDE;
    bool validate(const QString& word) const Q_DECL_OVERRIDE;
    QStringList suggestions(const QString& word, int count) const Q_DECL_OVERRIDE;
    void append(const QString& word) Q_DECL_OVERRIDE;
    void remove(const QString& word) Q_DECL_OVERRIDE;
    void ignore(const QString& word) Q_DECL_OVERRIDE;
    QStringList supportedLanguages() const Q_DECL_OVERRIDE;

    static bool isAvailable();
private:
    QScopedPointer<class HunspellBackendPrivate> d;
};
