#include <QtGlobal>
#include "qttablemodelexporterfactory.h"
#include "qttablemodelexporterplugin.h"
#include "qttablemodelexporter.h"

class QtTableModelExporterFactoryPrivate
{
public:
    typedef QHash<QString, QtTableModelExporterPlugin*> CreatorHash;
    CreatorHash creatorsMap;
};


QtTableModelExporterFactory::QtTableModelExporterFactory()
    : d(new QtTableModelExporterFactoryPrivate)
{
}

QtTableModelExporterFactory::~QtTableModelExporterFactory()
{
    qDeleteAll(d->creatorsMap);
}

void QtTableModelExporterFactory::registerExporter(QtTableModelExporterPlugin* plugin)
{
    if (plugin)
        d->creatorsMap[plugin->exporterName()] = plugin;
}

QtTableModelExporter* QtTableModelExporterFactory::createExporter(const QString& exporter,
                                                                  QAbstractTableModel* model) const
{
    auto it = d->creatorsMap.constFind(exporter);
    if (it == d->creatorsMap.cend())
        return Q_NULLPTR;

    return (*it)->create(model);
}

QIcon QtTableModelExporterFactory::exporterIcon( const QString& exporter ) const
{
    auto it = d->creatorsMap.constFind(exporter);
    if (it == d->creatorsMap.cend())
        return {};

    return (*it)->icon();
}

QStringList QtTableModelExporterFactory::keys() const
{
    return d->creatorsMap.keys();
}

QtTableModelExporterFactory& QtTableModelExporterFactory::instance()
{
    static QtTableModelExporterFactory globalInstance;
    return globalInstance;
}



