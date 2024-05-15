#include "qtspellcheckbackendfactory.h"
#include "qtspellcheckbackendplugin.h"
#include <QHash>

class QtSpellCheckBackendFactoryPrivate
{
public:
    typedef QHash<QString, QtSpellCheckBackendPlugin*> CreatorsHash;
    CreatorsHash creatorsMap;
};

QtSpellCheckBackendFactory::QtSpellCheckBackendFactory()
    : d(new QtSpellCheckBackendFactoryPrivate)
{
}

QtSpellCheckBackendFactory::~QtSpellCheckBackendFactory()
{
    qDeleteAll(d->creatorsMap);
}

QString QtSpellCheckBackendFactory::platformBackend() const
{
#ifdef Q_OS_LINUX
    return "Hunspell";
#elif Q_OS_WIN
    return "Win32Spell";
#elif Q_OS_MACOS
    return "NSSpell";
#else
    return {};
#endif
}

void QtSpellCheckBackendFactory::registerBackend(QtSpellCheckBackendPlugin* plugin)
{
    if (plugin)
        d->creatorsMap[plugin->backendName()] = plugin;
}

QtSpellCheckBackend* QtSpellCheckBackendFactory::createBackend(const QString& backendName) const
{
    auto it = d->creatorsMap.constFind(backendName);
    if (it == d->creatorsMap.cend())
        return Q_NULLPTR;

    return (*it)->create();
}

QStringList QtSpellCheckBackendFactory::keys() const
{
    return d->creatorsMap.keys();
}

QtSpellCheckBackendFactory& QtSpellCheckBackendFactory::instance()
{
    static QtSpellCheckBackendFactory globalInstance;
    return globalInstance;
}
