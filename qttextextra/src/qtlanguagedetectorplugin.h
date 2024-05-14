#pragma once
#include <QtPlugin>
#include <QtTextExtra>

class QtLanguageDetector;
class QTTEXTEXTRA_EXPORT QtLanguageDetectorPlugin
{
public:
    virtual ~QtLanguageDetectorPlugin(){}
    virtual QtLanguageDetector* create() const = 0;
    virtual QString backendName() const = 0;
};
Q_DECLARE_INTERFACE(QtLanguageDetectorPlugin, "com.QtExtra.QtLanguageDetectorPlugin/1.0")
