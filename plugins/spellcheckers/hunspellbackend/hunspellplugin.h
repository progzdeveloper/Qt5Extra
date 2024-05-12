#pragma once
#include <QObject>
#include <QtSpellCheckBackendPlugin>

class HunspellBackendPlugin :
        public QObject,
        public QtSpellCheckBackendPlugin
{
    Q_OBJECT
    Q_CLASSINFO("Version", "1.0")
    Q_CLASSINFO("HunspellBackendPlugin", "Hunspell Spell Check")

    Q_INTERFACES(QtSpellCheckBackendPlugin)

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    Q_PLUGIN_METADATA(IID "com.QtExtra.QtSpellCheckBackendPlugin/1.0" FILE "hunspellbackend.json")
#endif

public:
    explicit HunspellBackendPlugin(QObject *parent = Q_NULLPTR);

    // QtSpellCheckBackendPlugin interface
    QtSpellCheckBackend* create() const Q_DECL_OVERRIDE;
    QString backendName() const Q_DECL_OVERRIDE;
};
