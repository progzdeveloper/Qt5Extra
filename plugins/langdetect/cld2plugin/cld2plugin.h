#pragma once
#include <QObject>
#include <QtLanguageDetectorPlugin>

class CLD2LangDetectPlugin :
        public QObject,
        public QtLanguageDetectorPlugin
{
    Q_OBJECT
    Q_CLASSINFO("PluginVersion", "1.0")
    Q_CLASSINFO("CLD2LangDetectPlugin", "CLD2 Language Detector Plugin")

    Q_INTERFACES(QtLanguageDetectorPlugin)

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    Q_PLUGIN_METADATA(IID "com.QtExtra.CLD2LangDetectPlugin/1.0" FILE "cld2plugin.json")
#endif

public:
    explicit CLD2LangDetectPlugin(QObject *parent = Q_NULLPTR);

    // QtLanguageDetectorPlugin interface
    virtual QtLanguageDetector* create() const Q_DECL_OVERRIDE;
    virtual QString backendName() const Q_DECL_OVERRIDE;
};
