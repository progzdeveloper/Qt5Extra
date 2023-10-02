#include "qtrevertibleproxymodel.h"
#include <QMultiHash>

class QtRevertibleProxyModelPrivate
{
public:
    struct ItemData
    {
        QVariant value;
        int role;
        ItemData(const QVariant& v, int r) :
            value(v), role(r) {
        }
    };

    QHash<QPersistentModelIndex, ItemData> cache;

    inline void cacheIndex(const QModelIndex& index, const QVariant& value, int role);
    inline void revertModel(QAbstractItemModel* model);
};

inline void QtRevertibleProxyModelPrivate::cacheIndex(const QModelIndex &index, const QVariant &value, int role)
{
    cache.insertMulti(index, ItemData(value, role));
}

inline void QtRevertibleProxyModelPrivate::revertModel(QAbstractItemModel *model)
{
    for (auto it = cache.begin(); it != cache.end(); ++it) {
        model->setData(it.key(), (*it).value, (*it).role);
    }
}


QtRevertibleProxyModel::QtRevertibleProxyModel(QObject *parent)
    : QIdentityProxyModel(parent)
    , d(new QtRevertibleProxyModelPrivate)
{
}

QtRevertibleProxyModel::~QtRevertibleProxyModel()
{
}

bool QtRevertibleProxyModel::hasUncommitedChanges() const
{
     
    return (!d->cache.empty());
}

int QtRevertibleProxyModel::cacheSize() const
{
     
    return d->cache.size();
}

void QtRevertibleProxyModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    submit();
    QIdentityProxyModel::setSourceModel(sourceModel);
}

bool QtRevertibleProxyModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
     
    QModelIndex idx = mapToSource(index);
    QVariant val = idx.data(role);
    if (QIdentityProxyModel::setData(index, value, role)) {
        d->cacheIndex(idx, val, role);
        Q_EMIT changesCached(index, role);
        return true;
    }
    return false;
}

bool QtRevertibleProxyModel::submit()
{
     
    d->cache.clear();
    Q_EMIT accepted();
    return true;
}

void QtRevertibleProxyModel::revert()
{
     
    d->revertModel(sourceModel());
    d->cache.clear();
    Q_EMIT rejected();
}

bool QtRevertibleProxyModel::insertRows(int row, int count, const QModelIndex &parent)
{
    if (hasUncommitedChanges())
        return false;
    return QIdentityProxyModel::insertRows(row, count, parent);
}

bool QtRevertibleProxyModel::insertColumns(int column, int count, const QModelIndex &parent)
{
    if (hasUncommitedChanges())
        return false;
    return QIdentityProxyModel::insertColumns(column, count, parent);
}

bool QtRevertibleProxyModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (hasUncommitedChanges())
        return false;
    return QIdentityProxyModel::removeRows(row, count, parent);
}

bool QtRevertibleProxyModel::removeColumns(int column, int count, const QModelIndex &parent)
{
    if (hasUncommitedChanges())
        return false;
    return QIdentityProxyModel::removeColumns(column, count, parent);
}

