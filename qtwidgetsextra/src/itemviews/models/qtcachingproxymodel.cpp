#include <vector>
#include <unordered_map>
#include <QBasicTimer>
#include <QTimerEvent>
#include "qtcachingproxymodel.h"

class QtCachingProxyModelPrivate
{
public:
    mutable std::unordered_map<quint64, QVariant> cache;
    std::vector<int> cachedRoles;
    size_t maxCacheSize;
    int column;
    int interval;
    QBasicTimer timer;
    QtCachingProxyModel::CachingPolicy policy;

    QtCachingProxyModelPrivate()
        : maxCacheSize(100)
        , column(0)
        , interval(10000)
        , policy(QtCachingProxyModel::ManualUpdate)
    {}

    inline QVariant cacheValue(int row, int role, const QVariant& value) const
    {
        if (cache.size() > maxCacheSize)
            cache.erase(cache.begin());

        cache.emplace(indexate(row, role), value);
        return value;
    }

    inline bool isCachedRole(int role) const
    {
        return std::find(cachedRoles.begin(), cachedRoles.end(), role) != cachedRoles.end();
    }

    inline quint64 indexate(int row, int role) const
    {
        return ((quint64)row) | (((quint64)role) << 32);
    }
};


QtCachingProxyModel::QtCachingProxyModel(QObject *parent)
    : QIdentityProxyModel(parent)
    , d(new QtCachingProxyModelPrivate)
{
}

QtCachingProxyModel::~QtCachingProxyModel()
{
}

void QtCachingProxyModel::setCachingPolicy(QtCachingProxyModel::CachingPolicy policy)
{
     
    if (d->policy == policy)
        return;

    d->policy = policy;
    if (d->policy == AutoUpdate)
        d->timer.start(d->interval, this);
    else
        d->timer.stop();
}

QtCachingProxyModel::CachingPolicy QtCachingProxyModel::cachingPolicy() const
{
     
    return d->policy;
}

void QtCachingProxyModel::setUpdateInterval(int interval)
{
     
    if (d->interval == interval)
        return;

    d->interval = interval;
    if (d->timer.isActive())
    {
        d->timer.stop();
        d->timer.start(d->interval, this);
    }
}

int QtCachingProxyModel::updateInterval() const
{
     
    return d->interval;
}

void QtCachingProxyModel::setMaxCacheSize(int maxSize)
{
     
    if (maxSize < 0)
        maxSize = INT_MAX;
    d->maxCacheSize = static_cast<size_t>(maxSize);
    while (d->cache.size() > d->maxCacheSize)
        d->cache.erase(d->cache.begin()); // shrink cache to fit
}

int QtCachingProxyModel::maxCacheSize() const
{
     
    return d->maxCacheSize;
}

int QtCachingProxyModel::cacheSize() const
{
     
    return static_cast<int>(d->cache.size());
}

void QtCachingProxyModel::setCachedColumn(int column)
{
     
    if (d->column == column)
        return;

    clearCache();
    d->column = column;
}

int QtCachingProxyModel::cachedColumn() const
{
     
    return d->column;
}

void QtCachingProxyModel::setCachedRoles(const std::vector<int> &roles)
{
     
    if (d->cachedRoles == roles)
        return;

    clearCache();
    d->cachedRoles = roles;
}

const std::vector<int>& QtCachingProxyModel::cachedRoles() const
{
     
    return d->cachedRoles;
}

void QtCachingProxyModel::setSourceModel(QAbstractItemModel *model)
{
     
    if (sourceModel() != model)
        clearCache();

    QIdentityProxyModel::setSourceModel(model);
}

bool QtCachingProxyModel::setData(const QModelIndex &proxyIndex, const QVariant &value, int role)
{
     
    if (QIdentityProxyModel::setData(proxyIndex, value, role))
    {
        const QVariant value = proxyIndex.data(role);
        const quint64 idx = d->indexate(proxyIndex.row(), role);
        auto it = d->cache.find(idx);
        if (it != d->cache.end()) // cache hit
            it->second = value;
        else // cache miss
            d->cacheValue(proxyIndex.row(), role, value);
        return true;
    }
    return false;
}

QVariant QtCachingProxyModel::data(const QModelIndex &proxyIndex, int role) const
{
     
    if (proxyIndex.isValid() && proxyIndex.column() == d->column && d->isCachedRole(role))
    {
        const quint64 idx = d->indexate(proxyIndex.row(), role);
        auto it = d->cache.find(idx);
        if (it != d->cache.end()) // cache hit
            return it->second;
        else // cache miss
            return d->cacheValue(proxyIndex.row(), role, QIdentityProxyModel::data(proxyIndex, role));
    }
    return QIdentityProxyModel::data(proxyIndex, role);
}

void QtCachingProxyModel::clearCache()
{
     
    d->cache.clear();
}

void QtCachingProxyModel::updateCache()
{
    // basic implementation does nothing
}

void QtCachingProxyModel::cacheIndex(const QModelIndex &index)
{
     
    if (index.isValid() && index.column() == d->column)
    {
        for (auto role : d->cachedRoles)
            d->cacheValue(index.row(), role, index.data(role));
    }
}

void QtCachingProxyModel::timerEvent(QTimerEvent *event)
{
     
    if (event->timerId() == d->timer.timerId())
        updateCache();

    QIdentityProxyModel::timerEvent(event);
}
