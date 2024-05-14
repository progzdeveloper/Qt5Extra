#pragma once
#include <QFactoryInterface>
#include <QtSpellChecking>

class QtSpellCheckBackend;
class QtSpellCheckBackendPlugin;
class QTSPELLCHECKING_EXPORT QtSpellCheckBackendFactory :
        public QFactoryInterface
{
    Q_DISABLE_COPY(QtSpellCheckBackendFactory)
    QtSpellCheckBackendFactory();

public:
    ~QtSpellCheckBackendFactory();

    QString platformBackend() const;

    void registerBackend(QtSpellCheckBackendPlugin* plugin);
    QtSpellCheckBackend* createBackend(const QString& backendName) const;
    QStringList keys() const Q_DECL_OVERRIDE;

    static QtSpellCheckBackendFactory& instance();

private:
    QScopedPointer<class QtSpellCheckBackendFactoryPrivate> d;
};
