#include <QModelIndex>
#include "qtitemmodelsegregator.h"

namespace
{
    // A dirty little hack to get access to the
    // protected members in QAbstractItemModel
    class ItemModel : public QAbstractItemModel
    {
    public:
        using QAbstractItemModel::beginResetModel;
        using QAbstractItemModel::endResetModel;
        using QAbstractItemModel::beginInsertRows;
        using QAbstractItemModel::endInsertRows;
        using QAbstractItemModel::beginRemoveRows;
        using QAbstractItemModel::endRemoveRows;
    };
}


void QtItemModelSegregator::beginReset(QAbstractItemModel* m)
{
    ItemModel* q = static_cast<ItemModel*>(m);
    q->beginResetModel();
}

void QtItemModelSegregator::endReset(QAbstractItemModel* m)
{
    ItemModel* q = static_cast<ItemModel*>(m);
    q->endResetModel();
}

void QtItemModelSegregator::beginInsert(QAbstractItemModel* m, const QModelIndex& parent, int first, int last)
{
    ItemModel* q = static_cast<ItemModel*>(m);
    const int n = m->rowCount(parent);
    first = std::clamp(first, 0, std::max(0, n));
    last = std::max(first, last);
    q->beginInsertRows(parent, first, last);
}

void QtItemModelSegregator::endInsert(QAbstractItemModel* m)
{
    ItemModel* q = static_cast<ItemModel*>(m);
    q->endInsertRows();
}

void QtItemModelSegregator::beginRemove(QAbstractItemModel* m, const QModelIndex& parent, int first, int last)
{
    ItemModel* q = static_cast<ItemModel*>(m);
    const int n = m->rowCount(parent);
    first = std::clamp(first, 0, std::max(0, n - 1));
    last = std::clamp(last, 0, std::max(0, n - 1));
    q->beginRemoveRows(parent, first, last);
}

void QtItemModelSegregator::endRemove(QAbstractItemModel* m)
{
    ItemModel* q = static_cast<ItemModel*>(m);
    q->endRemoveRows();
}

void QtItemModelSegregator::notifyChanged(QAbstractItemModel* m, const QModelIndex& parent, int row, int column, std::initializer_list<int> roles)
{
    const QModelIndex index = m->index(row, column, parent);
    Q_EMIT m->dataChanged(index, index, roles);
}

