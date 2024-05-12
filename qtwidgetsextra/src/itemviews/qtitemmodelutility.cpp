#include "qtitemmodelutility.h"
#include <stack>

QString formatIndexPath(const QModelIndex &current, const QString &separator, int role)
{
    QModelIndex index = current;
    QString result;
    std::stack<QString> stack;
    int size = 0;
    while(index.isValid())
    {
        QString s = index.data(role).toString();
        size += s.size();
        stack.push(s);
        index = index.parent();
    }

    result.reserve(size + separator.size() * size);
    while(!stack.empty())
    {
        result += separator;
        result += stack.top();
        stack.pop();
    }
    return result;
}

QModelIndex findSourceIndex(const QModelIndex &index)
{
    using ConstProxyModel = const QAbstractProxyModel*;

    QModelIndex srcIdx = index;
    auto proxy = qobject_cast<ConstProxyModel>(index.model());
    while(proxy) // traverse through all proxies in bottom-up fasion
    {
        srcIdx = proxy->mapToSource(srcIdx);
        if (!srcIdx.isValid())
            return srcIdx; // break if we've got an invalid index

        proxy = qobject_cast<ConstProxyModel>(proxy->sourceModel());
    }
    return srcIdx;
}

const QAbstractItemModel* findSourceModel(const QModelIndex &index)
{
    return findSourceModel(index.model());
}

const QAbstractItemModel *findSourceModel(const QAbstractItemModel *model)
{
    using ConstProxyModel = const QAbstractProxyModel*;

    auto proxy = qobject_cast<ConstProxyModel>(model);
    while(proxy != Q_NULLPTR)
    {
        model = proxy->sourceModel();
        proxy = qobject_cast<ConstProxyModel>(model);
    }
    return model;
}

