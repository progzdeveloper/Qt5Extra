#pragma once
#include <QFactoryInterface>
#include <QString>
#include <QHash>
#include <QObject>
#include <QIcon>

#include <QtWidgetsExtra>

class QAbstractTableModel;
class QtTableModelExporter;
class QtTableModelExporterPlugin;

class QTWIDGETSEXTRA_EXPORT QtTableModelExporterFactory :
        public QFactoryInterface
{
    Q_DISABLE_COPY(QtTableModelExporterFactory)
    QtTableModelExporterFactory();
public:

    ~QtTableModelExporterFactory();

    void registerExporter(QtTableModelExporterPlugin* creator);
    QtTableModelExporter* createExporter(const QString& exporter,
                                         QAbstractTableModel* model) const;

    QIcon exporterIcon(const QString& exporter) const;

    QStringList keys() const Q_DECL_OVERRIDE;

    static QtTableModelExporterFactory& instance();

private:
    QScopedPointer<class QtTableModelExporterFactoryPrivate> d;
};
