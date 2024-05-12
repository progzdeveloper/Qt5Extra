#pragma once
#include <QtPlugin>
#include <QtSpellChecking>

class QtSpellCheckBackend;
class QTSPELLCHECKING_EXPORT QtSpellCheckBackendPlugin
{
public:
    virtual ~QtSpellCheckBackendPlugin(){}
    virtual QtSpellCheckBackend* create() const = 0;
    virtual QString backendName() const = 0;
};
Q_DECLARE_INTERFACE(QtSpellCheckBackendPlugin, "com.QtExtra.QtSpellCheckBackendPlugin/1.0")
