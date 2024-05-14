#include "enchantplugin.h"
#include "enchantbackend.h"

EnchantBackendPlugin::EnchantBackendPlugin(QObject* parent)
    : QObject(parent)
{
}

QtSpellCheckBackend* EnchantBackendPlugin::create() const
{
    return new EnchantBackend;
}

QString EnchantBackendPlugin::backendName() const
{
    return "Enchant";
}


#if QT_VERSION < 0x050000
#ifdef _DEBUG
Q_EXPORT_PLUGIN2("enchantbackendd", HunspellBackendPlugin)
#else
Q_EXPORT_PLUGIN2("enchantbackend", HunspellBackendPlugin)
#endif
#endif
