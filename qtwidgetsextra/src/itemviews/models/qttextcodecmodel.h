#pragma once
#include <QAbstractListModel>

#include <QtWidgetsExtra>

class QTWIDGETSEXTRA_EXPORT QtTextCodecModel :
        public QAbstractListModel
{
    Q_OBJECT

public:
    explicit QtTextCodecModel(QObject *parent = Q_NULLPTR);
    ~QtTextCodecModel();

    int rowCount ( const QModelIndex & parent = QModelIndex() ) const;

    QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;

private:
    QScopedPointer<class QtTextCodecModelPrivate> d;
};
