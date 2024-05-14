#pragma once
#include <QtSpellCheckBackend>
#include <QScopedPointer>

class EnchantBackend : public QtSpellCheckBackend
{
    // QtSpellCheckBackend interface
public:
    EnchantBackend();
    ~EnchantBackend() Q_DECL_OVERRIDE;
    bool load() Q_DECL_OVERRIDE;
    bool unload() Q_DECL_OVERRIDE;
    bool validate(const QString& word, const QStringList& langs) const Q_DECL_OVERRIDE;
    QStringList suggestions(const QString& word, int count, const QStringList& langs) const Q_DECL_OVERRIDE;
    void append(const QString& word) Q_DECL_OVERRIDE;
    void remove(const QString& word) Q_DECL_OVERRIDE;
    void ignore(const QString& word) Q_DECL_OVERRIDE;
    QStringList supportedLanguages() const Q_DECL_OVERRIDE;
    QList<SpellingProvider> providers() const Q_DECL_OVERRIDE;
private:
    QScopedPointer<class EnchantBackendPrivate> d;
};
