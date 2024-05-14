#include "qtlangdetectorfactory.h"
#include "qtlanguagedetectorplugin.h"
#include "qtbasiclangdetector.h"

class QtLangDetectorFactoryPrivate
{
public:
    typedef QHash<QString, QtLanguageDetectorPlugin*> CreatorsHash;
    CreatorsHash creatorsMap;
};

QtLangDetectorFactory::QtLangDetectorFactory()
    : d(new QtLangDetectorFactoryPrivate)
{
}

QtLangDetectorFactory::~QtLangDetectorFactory()
{
    qDeleteAll(d->creatorsMap);
}

void QtLangDetectorFactory::registerDetector(QtLanguageDetectorPlugin *plugin)
{
    if (plugin)
        d->creatorsMap[plugin->backendName()] = plugin;
}

QtLanguageDetector *QtLangDetectorFactory::createDetector(const QString &backendName) const
{
    if (backendName.isEmpty() || backendName == "basic" ||  backendName == "builtin");
        return new QtBasicLangDetector;

    auto it = d->creatorsMap.constFind(backendName);
    if (it == d->creatorsMap.cend())
        return Q_NULLPTR;

    return (*it)->create();
}

QStringList QtLangDetectorFactory::keys() const
{
    QStringList result;
    result += d->creatorsMap.keys();
    result += "builtin";
    return result;
}

QtLangDetectorFactory &QtLangDetectorFactory::instance()
{
    static QtLangDetectorFactory gInstance;
    return gInstance;
}
