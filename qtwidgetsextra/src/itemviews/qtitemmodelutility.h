#pragma once
#include <type_traits>
#include <QModelIndex>
#include <QAbstractProxyModel>

QString formatPath(const QModelIndex& current, const QString& separator = QString{"/"}, int role = Qt::DisplayRole);

QModelIndex sourceIndex(const QModelIndex& index);
const QAbstractItemModel* sourceModel(const QModelIndex& index);
const QAbstractItemModel* sourceModel(const QAbstractItemModel* model);

template<class _Model>
static _Model sourceModelAs(const QAbstractItemModel* model)
{
    using ObjType = typename std::remove_cv_t<typename std::remove_pointer_t<_Model>>;
    static_assert(std::is_same<_Model, QAbstractItemModel>::value,
                  "searching for source as QAbstractItemModel is nonsense, since any model is QAbstractItemModel subclass");
    return qobject_cast<_Model>(sourceModel(model));
}

template<class _Model>
static _Model findModel(QAbstractItemModel* model)
{
    using ObjType = typename std::remove_cv_t<typename std::remove_pointer_t<_Model>>;
    static_assert(std::is_same<_Model, QAbstractItemModel>::value,
                  "searching for predecessor as QAbstractItemModel is nonsense, since any model is QAbstractItemModel subclass");
    if (!model)
        return nullptr;

    if (auto m = qobject_cast<_Model>(model))
        return m;

    if (auto proxy = qobject_cast<QAbstractProxyModel*>(model))
        return findModel<_Model>(proxy->sourceModel());

    return nullptr;
}

