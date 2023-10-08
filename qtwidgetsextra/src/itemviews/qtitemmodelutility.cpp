#include "qtitemmodelutility.h"
#include <stack>

QString formatPath(const QModelIndex &current, const QString &separator, int role)
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

QModelIndex sourceIndex(const QModelIndex &index)
{
    const QAbstractProxyModel* proxy = qobject_cast<const QAbstractProxyModel*>(index.model());
    return  (proxy != Q_NULLPTR ? sourceIndex(proxy->mapToSource(index)) : index);
}

const QAbstractItemModel* sourceModel(const QModelIndex &index)
{
    return sourceModel(index.model());
}

const QAbstractItemModel *sourceModel(const QAbstractItemModel *model)
{
    const QAbstractProxyModel* proxy = qobject_cast<const QAbstractProxyModel*>(model);
    while(proxy != Q_NULLPTR)
    {
        model = proxy->sourceModel();
        proxy = qobject_cast<const QAbstractProxyModel*>(model);
    }
    return model;
}

