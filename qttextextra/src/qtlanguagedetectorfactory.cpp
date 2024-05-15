#include "qtlanguagedetectorfactory.h"
#include "qtlanguagedetectorplugin.h"
#include "qtbasiclangdetector.h"

class QtLangDetectorFactoryPrivate
{
public:
    typedef QHash<QString, QtLanguageDetectorPlugin*> CreatorsHash;
    CreatorsHash creatorsMap;
};

QtLanguageDetectorFactory::QtLanguageDetectorFactory()
    : d(new QtLangDetectorFactoryPrivate)
{
}

QtLanguageDetectorFactory::~QtLanguageDetectorFactory()
{
    qDeleteAll(d->creatorsMap);
}

void QtLanguageDetectorFactory::registerDetector(QtLanguageDetectorPlugin *plugin)
{
    if (plugin)
        d->creatorsMap[plugin->backendName()] = plugin;
}

QtLanguageDetector *QtLanguageDetectorFactory::createDetector(const QString &backendName) const
{
    if (backendName.isEmpty() || backendName == "basic" ||  backendName == "builtin")
        return new QtBasicLangDetector;

    auto it = d->creatorsMap.constFind(backendName);
    if (it == d->creatorsMap.cend())
        return Q_NULLPTR;

    return (*it)->create();
}

QStringList QtLanguageDetectorFactory::keys() const
{
    QStringList result;
    result += d->creatorsMap.keys();
    result += "builtin";
    return result;
}

QtLanguageDetectorFactory &QtLanguageDetectorFactory::instance()
{
    static QtLanguageDetectorFactory gInstance;
    return gInstance;
}
