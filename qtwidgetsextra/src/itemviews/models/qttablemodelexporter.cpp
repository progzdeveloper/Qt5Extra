#include <QtGlobal>
#include <QProgressDialog>
#include <QApplication>
#include <QTextCodec>

#include "qttablemodelexporter.h"
#include "qttablemodelexporterdialog.h"


QT_METAINFO_TR(QtTableModelExporter)
{
    QT_TR_META(QTableModelExporter, "Common"), // Общие
    QT_TR_META(QTableModelExporter, "Header"), // Заголовок
    QT_TR_META(QTableModelExporter, "Column names") // Записать столбцы
};

#undef QT_META_TR

class QtTableModelExporterPrivate
{
public:
    QtTableModelExporterPrivate(QAbstractTableModel* m) :
        model(m),
        tableName("[Title]"),
        codec(QTextCodec::codecForLocale()),
        role(Qt::DisplayRole),
        storeHeader(false)
    {
    }

    QAbstractTableModel *model;
    QString tableName;
    QString errorString;
    QTextCodec *codec;
    int role;
    bool storeHeader;
    QProgressDialog *dlg;
};


QtTableModelExporter::QtTableModelExporter(QAbstractTableModel* model)
    : d(new QtTableModelExporterPrivate(model))
{
    setModel(model);
}

QtTableModelExporter::~QtTableModelExporter() = default;

void QtTableModelExporter::setModel(QAbstractTableModel* model)
{
     
    d->model = model;
}

QAbstractTableModel* QtTableModelExporter::model() const
{
     
    return d->model;
}

void QtTableModelExporter::setTextCodec( QTextCodec* codec )
{
     
    if (codec)
        d->codec = codec;
}

QTextCodec* QtTableModelExporter::textCodec() const
{
     
    return d->codec;
}

void QtTableModelExporter::setItemRole( int role /*= Qt::DisplayRole*/ )
{
     
    d->role = role;
}

int QtTableModelExporter::itemRole() const
{
     
    return d->role;
}

QDialog *QtTableModelExporter::createDialog(QWidget *parent) const
{
    return new QtTableModelExporterDialog(const_cast<QtTableModelExporter*>(this), parent);
}

void QtTableModelExporter::setHeaderStored(bool on)
{
     
    d->storeHeader = on;
}

bool QtTableModelExporter::isHeaderStored() const
{
     
    return d->storeHeader;
}

void QtTableModelExporter::setTableName(const QString& name)
{
     
    d->tableName = name;
}

QString QtTableModelExporter::tableName() const
{
     
    return d->tableName;
}

QString QtTableModelExporter::errorString() const
{
     
    return d->errorString;
}

QStringList QtTableModelExporter::fileFilter() const
{
    return QStringList();
}

bool QtTableModelExporter::exportModel(QIODevice * /*device*/)
{
    return false;
}

void QtTableModelExporter::setErrorString( const QString& text )
{
     
    d->errorString = text;
}

void QtTableModelExporter::setProgress( int step )
{
     
    if (d->dlg) {
        d->dlg->setValue(step);
    }
    qApp->processEvents();
}

void QtTableModelExporter::setProgressText( const QString& text )
{
     
    if (d->dlg) {
        d->dlg->setLabelText(text);
    }
    qApp->processEvents();
}

bool QtTableModelExporter::beginExport(QIODevice *device)
{
     
    if (!d->model) {
        d->errorString = tr("source data model is not set");
        return false;
    }

    if (!device) {
        d->errorString = tr("output device is not presented");
        return false;
    }

    if (!device->isOpen() || !device->isWritable()) {
        d->errorString = tr("output device is inaccessible");
        return false;
    }

    d->dlg = new QProgressDialog(tr("Data export..."), tr("Cancel"), 0,
                                 d->model->rowCount() * d->model->columnCount());
    d->dlg->setAttribute(Qt::WA_DeleteOnClose);
    d->dlg->setAutoClose(false);
    d->dlg->show();
    qApp->processEvents();
    return true;
}

void QtTableModelExporter::endExport()
{
     
    d->dlg->setValue(d->dlg->maximum());
    d->dlg->close();
    qApp->processEvents();

}

bool QtTableModelExporter::aborted() const
{
     
    qApp->processEvents();
    return d->dlg->wasCanceled();
}



