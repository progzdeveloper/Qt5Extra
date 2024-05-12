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

    void spell(const QString& _word, int _offset, QObject* _receiver = nullptr);
    void append(const QString& _word);
    void remove(const QString& _word);
    void ignore(const QString& _word);
    void requestSuggests(const QString& _word, int _count, QObject* _receiver = nullptr);
    void cancel(QObject* _object);

Q_SIGNALS:
    void misspelled(QObject* _object, const QString& _word, int _offset, bool _needMarkAsMisspelled = true);
    void completed(QObject* _object);
    void appended(const QString& _word);
    void removed(const QString& _word);
    void ignored(const QString& _word);
    void suggestsFound(QObject* _receiver, const QString& _word, const QStringList& results, QtSpellCheckEngine::CorrectionActions _actions);

private:
    void run() Q_DECL_OVERRIDE;

private:
    QScopedPointer<class QtSpellCheckEnginePrivate> d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QtSpellCheckEngine::CorrectionActions)
Q_DECLARE_METATYPE(QtSpellCheckEngine::CorrectionActions)
