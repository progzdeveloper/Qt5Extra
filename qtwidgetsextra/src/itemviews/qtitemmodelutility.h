#pragma once
#include <type_traits>
#include <QModelIndex>
#include <QAbstractProxyModel>

QString formatPath(const QModelIndex& current, const QString& separator = QString{"/"}, int role = Qt::DisplayRole);

QModelIndex sourceIndex(const QModelIndex& index);
const QAbstractItemModel* sourceModel(const QModelIndex& index);
const QAbstractItemModel* sourceModel(const QAbstractItemModel* model);

template<class _Model>
static _Model sourceItemModel(const QAbstractItemModel* model)
{
    using ObjType = typename std::remove_cv_t<typename std::remove_pointer_t<_Model>>;
    if constexpr (!std::is_const_v<_Model>)
        return const_cast<ObjType*>(qobject_cast<const ObjType*>(sourceModel(model)));
    else
        return qobject_cast<const ObjType*>(sourceModel(model));
}

template<class _Model>
static _Model findModel(QAbstractItemModel* model)
{
    using ObjType = typename std::remove_cv_t<typename std::remove_pointer_t<_Model>>;
    static_assert(std::is_same_v<_Model, QAbstractItemModel>,
                  "searching for predecessor as QAbstractItemModel is nonsense, since any model is QAbstractItemModel subclass");
    if (!model)
        return Q_NULLPTR;

    if (auto m = qobject_cast<_Model>(model))
        return m;

    if (auto proxy = qobject_cast<QAbstractProxyModel*>(model))
        return findModel<_Model>(proxy->sourceModel());

    return Q_NULLPTR;
}

