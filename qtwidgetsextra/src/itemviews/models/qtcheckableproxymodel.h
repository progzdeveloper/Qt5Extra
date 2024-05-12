#pragma once
#include <QSortFilterProxyModel>

#include <QtWidgetsExtra>

class QTWIDGETSEXTRA_EXPORT QtCheckableProxyModel :
        public QSortFilterProxyModel
{
    typedef QSortFilterProxyModel BaseModel;

    Q_OBJECT
public:
    explicit QtCheckableProxyModel(QObject *parent = Q_NULLPTR);
    ~QtCheckableProxyModel();

    // QAbstractProxyModel interface
    void setSourceModel(QAbstractItemModel *model) Q_DECL_OVERRIDE;

    void setModelColumn(int column);
    int modelColumn() const;

    void setCheckState(Qt::CheckState state);

    Qt::CheckState checkState() const;

    // QAbstractItemModel interface
    bool setData(const QModelIndex &index, const QVariant &value, int role) Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;

    // QAbstractItemModel interface
    bool insertRows(int row, int count, const QModelIndex &parent) Q_DECL_OVERRIDE;
    bool removeRows(int row, int count, const QModelIndex &parent) Q_DECL_OVERRIDE;

public Q_SLOTS:
    void setChecked();
    void setUnchecked();
    void invert();

    void filterChecked();
    void clearFilter();

    void insert(const QModelIndex&, int row, int count);
    void remove(const QModelIndex&, int row, int count);
    void resetInternals();

Q_SIGNALS:
    void checkStateChanged(Qt::CheckState);

private:
    QScopedPointer<class QtCheckableProxyModelPrivate> d;
};
