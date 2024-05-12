#ifndef QOPTIONMODEL_H
#define QOPTIONMODEL_H

#include <QAbstractTableModel>

class QOptionModel :
    public QAbstractTableModel
{
    Q_OBJECT

public:
    enum ColumnType
    {
        ColumnExpr = 0,
        ColumnVal,
        ColumnImp,
        ColumnReq,
        ColumnText,
        COLUMN_COUNT = (ColumnText + 1)
    };


    explicit QOptionModel(QObject *parent = nullptr);

    bool insertRows(int row, int count, const QModelIndex &parent) Q_DECL_OVERRIDE;
    bool removeRows(int row, int count, const QModelIndex &parent) Q_DECL_OVERRIDE;
    bool moveRows(const QModelIndex &sourceParent,
                                      int sourceRow, int count,
                                      const QModelIndex &destinationParent, int destinationChild) Q_DECL_OVERRIDE;

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

    // Editable:
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) Q_DECL_OVERRIDE;

    Qt::ItemFlags flags(const QModelIndex& index) const Q_DECL_OVERRIDE;

    QModelIndex index(int row, int column, const QModelIndex &parent) const;

    // ###
    QString generateCppCode(const QString& className = "command_line") const;

public Q_SLOTS:
    void clear();

private:
    class QOptionModelPrivate* d_ptr;
    Q_DECLARE_PRIVATE(QOptionModel)
};

#endif // QOPTIONMODEL_H
