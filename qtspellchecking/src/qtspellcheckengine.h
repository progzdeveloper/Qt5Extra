#pragma once
#include <QThread>
#include <QSet>

#include <QtSpellChecking>

class QtSpellChecker;
class QtSpellCheckBackend;

class QTSPELLCHECKING_EXPORT QtSpellCheckEngine : public QThread
{
    Q_OBJECT
    Q_DISABLE_COPY(QtSpellCheckEngine)

    QtSpellCheckEngine();

public:
    enum SpellingAction
    {
        NoActions = 0,
        AppendWord = 1 << 0,
        IgnoreWord = 1 << 1,
        RemoveWord = 1 << 2
    };
    Q_DECLARE_FLAGS(SpellingActions, SpellingAction)
    Q_FLAG(SpellingActions)

    ~QtSpellCheckEngine() Q_DECL_OVERRIDE;

    static QtSpellCheckEngine& instance();

    void setPrefferedBackend(const QString& backend);
    QString preferredBackend() const;

    QString backendName() const;
    QStringList supportedLanguages() const;

    void spell(const QString& word, int offset, const QStringList& langs = {}, QObject* receiver = nullptr);
    void requestSuggests(const QString& word, int count, const QStringList& langs = {}, QObject* receiver = nullptr);
    void append(const QString& word);
    void remove(const QString& word);
    void ignore(const QString& word);
    void cancel(QObject* object);

Q_SIGNALS:
    void misspelled(QObject* object, const QString& word, int offset);
    void completed(QObject* object);
    void appended(const QString& word);
    void removed(const QString& word);
    void ignored(const QString& word);
    void suggestsFound(QObject* receiver, const QString& word, const QStringList& results, QtSpellCheckEngine::SpellingActions actions);

private:
    void run() Q_DECL_OVERRIDE;

private:
    QScopedPointer<class QtSpellCheckEnginePrivate> d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QtSpellCheckEngine::SpellingActions)
Q_DECLARE_METATYPE(QtSpellCheckEngine::SpellingActions)
