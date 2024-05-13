#pragma once
#include <QIdentityProxyModel>

#include <QtWidgetsExtra>

class QtAbstractItemFilter;

/*!
     * \brief The GroupItemDataDelegate class
     * \details Customize the appearance of particular group item
     */
class QTWIDGETSEXTRA_EXPORT QtGroupItemDataDelegate
{
public:
    /*!
     * \brief Release all resources and destroy GroupItemDelegate
     */
    virtual ~QtGroupItemDataDelegate() = default;
    /*!
     * \brief setData set item data
     * \param value value to set
     * \param role value role
     * \return true if successfull, otherwise return false
     */
    virtual bool setData(const QVariant& /*value*/, int /*role*/) { return false; }
    /*!
         * \brief data extract item data for specified role
         * \param role item data role
         * \return data value for role
         */
    virtual QVariant data(int /*role*/) const { return {}; }
    /*!
         * \brief match try to match item data aganist model index
         * \param index model index to match with
         * \return true if index data matched, otherwise return false
         */
    virtual bool match(const QModelIndex& index) const = 0;
    /*!
         * \brief filter pointer to current item filter
         * \return pointer to current filter
         */
    virtual QtAbstractItemFilter* filter() const { return nullptr; }
};


/*!
 * \brief The GroupingProxyModel class
 * The GroupingProxyModel is a QIdentityProxyModel based proxy model that
 * provide the ability to group items into categories by specified filter.
 * The source model become visible as a tree model for GroupingProxyModel
 * clients. To create group one must provide a AbstractItemFilter filter,
 * or GroupItemDataDelegate implementation for categorizing and grouping
 * items of source model. By default only single uncategorized group is
 * created. The placement and visibility of uncategorized items group can
 * be controlled with UngrouppedPlacement and UngrouppedPolicy respectivly.
 * Each group become a top-level item, while children items is items from
 * source model that satisfies the predicate specified for group.
 * At any time GroupingProxyModel can be converted to a QIdentityProxyModel
 * in-place by calling setIdentity(true) method.
 * \warning The refresh() method reset any previously calculated group
 * index mappings and rebuild them from scratch. To minimize resource
 * consumption it's recomended to avoid call this method not very often.
 */
class QTWIDGETSEXTRA_EXPORT QtGroupingProxyModel : public QIdentityProxyModel
{
    Q_OBJECT
    Q_PROPERTY(int groupColumn READ groupColumn WRITE setGroupColumn NOTIFY groupColumnChanged)
    Q_PROPERTY(bool groupsSpanned READ isGroupsSpanned WRITE setGroupsSpanned NOTIFY groupsSpannedChanged)
    Q_PROPERTY(bool identity READ isIdentity WRITE setIdentity NOTIFY identityChanged)
    Q_PROPERTY(UngrouppedPolicy ungrouppedPolicy READ ungrouppedPolicy WRITE setUngrouppedPolicy NOTIFY ungrouppedPolicyChanged)
    Q_PROPERTY(UngrouppedPlacement ungrouppedPlacement READ ungrouppedPlacement WRITE setUngrouppedPlacement NOTIFY ungrouppedPlacementChanged)

public:
    enum UngrouppedPolicy
    {
        UngrouppedAlwaysOff,
        UngrouppedAlwaysOn,
        UngrouppedAutoHide
    };
    Q_ENUM(UngrouppedPolicy)

    enum UngrouppedPlacement
    {
        UngrouppedAtFront,
        UngrouppedAtBack
    };
    Q_ENUM(UngrouppedPlacement)

    explicit QtGroupingProxyModel(QObject* _parent = nullptr);
    ~QtGroupingProxyModel();

    virtual void setSourceModel(QAbstractItemModel* _model) Q_DECL_OVERRIDE;

    void setIdentity(bool _on);
    bool isIdentity() const;

    void setGroupColumn(int _column);
    int groupColumn() const;

    void setGroupsSpanned(bool _on);
    bool isGroupsSpanned() const;

    void setUngrouppedPolicy(UngrouppedPolicy _policy);
    UngrouppedPolicy ungrouppedPolicy() const;

    void setUngrouppedPlacement(UngrouppedPlacement _placement);
    UngrouppedPlacement ungrouppedPlacement() const;

    QtGroupItemDataDelegate* group(const QString& _groupName, QtAbstractItemFilter* _filter);
    QtGroupItemDataDelegate* group(QtGroupItemDataDelegate* _delegate);
    void ungroup(const QString& name);
    void ungroup(const QtGroupItemDataDelegate* _delegate);
    void ungroup();

    QModelIndex groupIndex(const QtGroupItemDataDelegate* _delegate) const;
    QModelIndex groupIndex(const QString& _name) const;

    virtual QVariant ungroppedData(int _role) const;
    virtual bool setUngrouppedData(const QVariant& _value, int _role);

    // QAbstractItemModel interface
public:
    QModelIndex index(int _row, int _column, const QModelIndex& _parent = {}) const Q_DECL_OVERRIDE;
    QModelIndex parent(const QModelIndex& _child = {}) const Q_DECL_OVERRIDE;
    int rowCount(const QModelIndex& _parent = {}) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex& _parent = {}) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex& proxyIndex = {}, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    bool setData(const QModelIndex& proxyIndex, const QVariant& value, int role = Qt::EditRole) Q_DECL_OVERRIDE;
    Qt::ItemFlags flags(const QModelIndex& _proxyIndex) const Q_DECL_OVERRIDE;
    QModelIndex buddy(const QModelIndex& _index) const Q_DECL_OVERRIDE;

    QVariant headerData(int _section, Qt::Orientation _orientation, int _role) const Q_DECL_OVERRIDE;
    bool setHeaderData(int _section, Qt::Orientation _orientation, const QVariant& _value, int _role = Qt::EditRole) Q_DECL_OVERRIDE;

    // QAbstractProxyModel interface
public:
    QModelIndex mapToSource(const QModelIndex& proxyIndex) const Q_DECL_OVERRIDE;
    QModelIndex mapFromSource(const QModelIndex& sourceIndex) const Q_DECL_OVERRIDE;

public Q_SLOTS:
    void refresh();

private Q_SLOTS:
    void onSourceRowsInserted(const QModelIndex& _parent, int _first, int _last);
    void onSourceRowsRemoved(const QModelIndex& _parent, int _first, int _last);
    void onSourceRowsMoved(const QModelIndex& _parent, int _start, int _end, const QModelIndex& _destination, int _row);
    void onSourceDataChanged(const QModelIndex& _topLeft, const QModelIndex& _bottomRight, const QVector<int>& _roles);
    void onSourceReset();
    void onSourceDestroyed();

Q_SIGNALS:
    void groupColumnChanged(int);
    void groupsSpannedChanged(bool);
    void identityChanged(bool);
    void ungrouppedPolicyChanged(QtGroupingProxyModel::UngrouppedPolicy);
    void ungrouppedPlacementChanged(QtGroupingProxyModel::UngrouppedPlacement);

private:
    friend class QtGroupingProxyModelPrivate;
    QScopedPointer<class QtGroupingProxyModelPrivate> d;
};
