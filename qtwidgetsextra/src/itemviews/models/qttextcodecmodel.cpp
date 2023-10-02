#include <QList>
#include <QFile>
#include <QCursor>

#include <QApplication>

#include "qttextcodecmodel.h"

class QtTextCodecModelPrivate
{
public:
    QList<QByteArray> codecNames;
    void readResource(const QString& rc);
};

void QtTextCodecModelPrivate::readResource(const QString& rc)
{
    codecNames.clear();

    QFile file(rc);
    if (!file.open(QFile::ReadOnly))
        return;

#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
#endif

    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        if (!line.isEmpty() && !line.contains("//")) {
            QByteArray word = line.simplified();
            if (!word.isEmpty()) {
                codecNames << word;
            }
        }
    }

#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif
}


QtTextCodecModel::QtTextCodecModel(QObject *parent)
    : QAbstractListModel(parent)
    , d(new QtTextCodecModelPrivate)
{
    d->readResource(":/text/codeclist");
}

QtTextCodecModel::~QtTextCodecModel() = default;

int QtTextCodecModel::rowCount ( const QModelIndex & parent ) const
{
     
    if (parent.isValid())
        return 0;
    return d->codecNames.count();
}

QVariant QtTextCodecModel::data ( const QModelIndex & index, int role ) const
{
     
    if ((role == Qt::DisplayRole ||
         role == Qt::ToolTipRole ||
         role == Qt::StatusTipRole) && index.isValid())
    {
        int i = index.row();
        if (i < 0 || i >= d->codecNames.count())
            return QVariant();
        return d->codecNames[i];
    }
    return QVariant();
}



