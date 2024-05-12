#pragma once
#include <type_traits>
#include <QModelIndex>
#include <QAbstractProxyModel>

#include <QtWidgetsExtra>

QTWIDGETSEXTRA_EXPORT QString formatIndexPath(const QModelIndex& current, const QString& separator = QString{"/"}, int role = Qt::DisplayRole);
QTWIDGETSEXTRA_EXPORT QModelIndex findSourceIndex(const QModelIndex& index);
QTWIDGETSEXTRA_EXPORT const QAbstractItemModel* findSourceModel(const QModelIndex& index);
QTWIDGETSEXTRA_EXPORT const QAbstractItemModel* findSourceModel(const QAbstractItemModel* model);

template<class _Model>
static _Model findSourceModelAs(const QAbstractItemModel* model)
{
    using ObjType = typename std::remove_cv_t<typename std::remove_pointer_t<_Model>>;
    if constexpr (!std::is_const_v<_Model>)
        return const_cast<ObjType*>(qobject_cast<const ObjType*>(findSourceModel(model)));
    else
        return qobject_cast<const ObjType*>(findSourceModel(model));
}

template<class _Model>
static _Model findItemModelAs(QAbstractItemModel* model)
{
    using ObjType = typename std::remove_cv_t<typename std::remove_pointer_t<_Model>>;
    static_assert(std::is_same_v<_Model, QAbstractItemModel>,
                  "searching for predecessor as QAbstractItemModel is nonsense, since any model is QAbstractItemModel subclass");
    if (!model)
        return Q_NULLPTR;

    if (auto m = qobject_cast<const ObjType*>(model))
    {
        if constexpr (!std::is_const_v<_Model>)
            return const_cast<ObjType*>(m);
        else
            return m;
    }

    if (auto proxy = qobject_cast<QAbstractProxyModel*>(model))
        return findItemModelAs<_Model>(proxy->sourceModel());

    return Q_NULLPTR;
}

