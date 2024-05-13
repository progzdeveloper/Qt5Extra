#pragma once
#include <QThread>

#include <QtSpellChecking>

class QtSpellChecker;
class QtSpellCheckBackend;

class QTSPELLCHECKING_EXPORT QtSpellCheckEngine : public QThread
{
    Q_OBJECT
    Q_DISABLE_COPY(QtSpellCheckEngine)

    QtSpellCheckEngine();

public:
    enum CorrectionAction
    {
        NoActions = 0,
        AppendWord = 1 << 0,
        IgnoreWord = 1 << 1,
        RemoveWord = 1 << 2
    };
    Q_DECLARE_FLAGS(CorrectionActions, CorrectionAction)

    ~QtSpellCheckEngine() Q_DECL_OVERRIDE;

    static QtSpellCheckEngine& instance();

    void setPrefferedBackend(const QString& backend);
    QString preferredBackend() const;

    QString backendName() const;

    void spell(const QString& word, int offset, QObject* receiver = nullptr);
    void append(const QString& word);
    void remove(const QString& word);
    void ignore(const QString& word);
    void requestSuggests(const QString& word, int count, QObject* receiver = nullptr);
    void cancel(QObject* object);

Q_SIGNALS:
    void misspelled(QObject*_object, const QString& word, int offset);
    void completed(QObject* object);
    void appended(const QString& word);
    void removed(const QString& word);
    void ignored(const QString& word);
    void suggestsFound(QObject* receiver, const QString& word, const QStringList& results, QtSpellCheckEngine::CorrectionActions actions);

private:
    void run() Q_DECL_OVERRIDE;

private:
    QScopedPointer<class QtSpellCheckEnginePrivate> d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QtSpellCheckEngine::CorrectionActions)
Q_DECLARE_METATYPE(QtSpellCheckEngine::CorrectionActions)
