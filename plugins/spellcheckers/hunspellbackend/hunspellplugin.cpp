#include "hunspellplugin.h"
#include "hunspellbackend.h"

HunspellBackendPlugin::HunspellBackendPlugin(QObject* parent)
    : QObject(parent)
{
}

QtSpellCheckBackend* HunspellBackendPlugin::create() const
{
    return new HunspellBackend;
}

QString HunspellBackendPlugin::backendName() const
{
    return QStringLiteral("Hunspell");
}


#if QT_VERSION < 0x050000
#ifdef _DEBUG
Q_EXPORT_PLUGIN2("hunspellbackendd", HunspellBackendPlugin)
#else
Q_EXPORT_PLUGIN2("hunspellbackend", HunspellBackendPlugin)
#endif
#endif
