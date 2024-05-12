#include <QModelIndex>
#include <QVariant>
#include <QPointer>
#include <memory>
#include <unordered_map>
#include <deque>
#include <algorithm>

#include "qtitemfilter.h"
#include "qtgroupingproxymodel.h"

namespace
{
    static bool isChecked(const QModelIndex& _index)
    {
        const Qt::CheckState checkState = static_cast<Qt::CheckState>(_index.data(Qt::CheckStateRole).value<int>());
        return (checkState == Qt::Checked || checkState == Qt::PartiallyChecked);
    }

    class FilterGroupItemDelegate : public QtGroupItemDataDelegate
    {
    public:
        FilterGroupItemDelegate(const QString& _title, QtAbstractItemFilter* _filter)
            : title_(_title)
            , filter_(_filter)
        {
            Q_ASSERT(filter_ != Q_NULLPTR);
            filter_->setEnabled(true);
        }

        ~FilterGroupItemDelegate() = default;

        QtAbstractItemFilter* filter() const Q_DECL_OVERRIDE { return filter_.get(); }

        // QtGroupingDelegate interface
    public:
        QVariant data(int _role) const Q_DECL_OVERRIDE
        {
            if (_role == Qt::EditRole || _role == Qt::DisplayRole)
                return title_;

            auto it = values_.find(_role);
            return it != values_.end() ? it->second : QVariant{};
        }

        bool setData(const QVariant& _value, int _role) Q_DECL_OVERRIDE
        {
            if (_role == Qt::EditRole || _role == Qt::DisplayRole)
            {
                title_ = _value.toString();
                return true;
            }
            if (_value.isValid())
                values_[_role] = _value;
            else
                values_.erase(_role);
            return true;
        }

        bool match(const QModelIndex& _index) const Q_DECL_OVERRIDE
        {
            return filter_ && filter_->accepted(_index);
        }

    private:
        QString title_;
        std::unordered_map<int, QVariant> values_;
        std::unique_ptr<QtAbstractItemFilter> filter_;
    };

    class CountingIterator
    {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = int;
        using reference = int&;
        using const_reference = const int&;
        using pointer = int*;
        using const_pointer = const int*;

        CountingIterator() = default;
        CountingIterator(int i) Q_DECL_NOTHROW : value(i){}

        int operator*() Q_DECL_NOTHROW{ return value; }
        pointer operator->() Q_DECL_NOTHROW { return &value; }

        int operator*() const Q_DECL_NOTHROW{ return value; }
        const_pointer operator->() const Q_DECL_NOTHROW { return &value; }

        CountingIterator& operator++() Q_DECL_NOTHROW { ++value; return *this; }
        CountingIterator operator++(int) Q_DECL_NOTHROW { auto t = *this; ++value; return t; }
        CountingIterator& operator--() Q_DECL_NOTHROW { --value; return *this; }
        CountingIterator operator--(int) Q_DECL_NOTHROW { auto t = *this; --value; return t; }

        bool operator== (const CountingIterator& i) const Q_DECL_NOTHROW { return value == i.value; }
        bool operator!= (const CountingIterator& i) const Q_DECL_NOTHROW { return value != i.value; }

    private:
        int value = 0;
    };
}



class QtGroupingProxyModelPrivate
{
public:
    struct ModelDataChangeLocker
    {
        explicit ModelDataChangeLocker(QtGroupingProxyModel* _proxy)
            : proxy_(_proxy)
        {
            if (QAbstractItemModel* model = proxy_->sourceModel())
                QObject::disconnect(model, &QAbstractItemModel::dataChanged, proxy_, &QtGroupingProxyModel::onSourceDataChanged);
        }

        ~ModelDataChangeLocker()
        {
            if (QAbstractItemModel* model = proxy_->sourceModel())
                QObject::connect(model, &QAbstractItemModel::dataChanged, proxy_, &QtGroupingProxyModel::onSourceDataChanged);
        }
    private:
        QtGroupingProxyModel* proxy_;
    };

    class GroupItem
    {
    public:
        explicit GroupItem(QtGroupItemDataDelegate* _delegate = nullptr)
            : delegate_(_delegate)
        {}

        bool operator== (const QtGroupItemDataDelegate* _delegate) const
        {
            return delegate_.get() == _delegate;
        }

        bool operator!= (const QtGroupItemDataDelegate* _delegate) const
        {
            return delegate_.get() != _delegate;
        }

        bool operator== (const QString& _name) const
        {
            return (delegate_ && delegate_->data(Qt::DisplayRole).toString() == _name);
        }

        bool operator!= (const QString& _name) const
        {
            return !((*this) == _name);
        }

        const QtGroupItemDataDelegate* delegate() const
        {
            return delegate_.get();
        }

        bool isEmpty() const
        {
            return sourceRowMap_.empty();
        }

        int childCount() const
        {
            return static_cast<int>(sourceRowMap_.size());
        }

        bool match(const QModelIndex& index) const
        {
            return (delegate_ && delegate_->match(index));
        }

        void clearFlags()
        {
            cachedFlags_ = Qt::NoItemFlags;
        }

        Qt::ItemFlags flags(const QtGroupingProxyModel* _proxy) const
        {
            const QAbstractItemModel* model = _proxy->sourceModel();
            if (!model)
                return Qt::NoItemFlags;

            if (cachedFlags_ != Qt::NoItemFlags)
                return cachedFlags_;

            Qt::ItemFlags itemFlags = Qt::ItemIsEnabled;
            for (int row : sourceRowMap_)
            {
                const QModelIndex index = model->index(row, _proxy->groupColumn());
                if (index.isValid())
                    itemFlags |= index.flags();
            }
            // update cached flags
            cachedFlags_ = itemFlags;
            return itemFlags;
        }

        QVariant data(int _role, const QtGroupingProxyModel* _proxy) const
        {
            if (_role == Qt::CheckStateRole && (cachedFlags_ & Qt::ItemIsUserCheckable))
            {
                if (checkedCount_ == 0)
                    return Qt::Unchecked;
                else if (childCount() == checkedCount_)
                    return Qt::Checked;
                else
                    return Qt::PartiallyChecked;
            }

            Q_ASSERT(_proxy != nullptr);
            if (_proxy->isGroupsSpanned())
                return (_proxy->groupColumn() >= 0 ? itemData(_role, _proxy) : QVariant{});
            else
                return itemData(_role, _proxy);
        }

        bool setData(const QVariant& _value, int _role, QtGroupingProxyModel* _proxy)
        {
            Q_ASSERT(_proxy != nullptr);

            if (!delegate_ && _role != Qt::CheckStateRole)
                return _proxy->setUngrouppedData(_value, _role);

            if (_role != Qt::CheckStateRole)
                return delegate_->setData(_value, _role);

            QAbstractItemModel* sourceModel = _proxy->sourceModel();
            if (!sourceModel)
                return false;

            const int column = _proxy->groupColumn();
            const int checked = static_cast<Qt::CheckState>(_value.toInt());

            ModelDataChangeLocker locker(_proxy);
            int successCount = 0;
            for (int row : sourceRowMap_)
            {
                const QModelIndex index = sourceModel->index(row, column);
                if (index.isValid())
                    successCount += sourceModel->setData(index, _value, _role);
            }
            checkedCount_ = checked ? successCount : childCount() - successCount;
            return true;
        }

        void clear()
        {
            sourceRowMap_.clear();
            checkedCount_ = 0;
        }

        void resetChecked(QtGroupingProxyModel* _proxy)
        {
            checkedCount_ = 0;
            QAbstractItemModel* model = _proxy->sourceModel();
            const int column = _proxy->groupColumn();
            for (int sourceRow : sourceRowMap_)
                checkedCount_ += isChecked(model->index(sourceRow, column));
            clearFlags();
        }

        bool push(QtGroupingProxyModel* _proxy, int _groupIdx, const QModelIndex& _sourceIndex, bool _matchRequired = true)
        {
#ifdef _DEBUG
            if (!sourceRowMap_.empty())
                Q_ASSERT(_sourceIndex.row() > sourceRowMap_.back());
#endif
            if (_matchRequired && !match(_sourceIndex))
                return false;

            const int i = static_cast<int>(sourceRowMap_.size());
            _proxy->beginInsertRows(_proxy->index(_groupIdx, 0), i, i);
            sourceRowMap_.emplace_back(_sourceIndex.row());
            _proxy->endInsertRows();
            checkedCount_ += isChecked(_sourceIndex);
            return true;
        }

        int update(const QModelIndex& _sourceIndex, bool _matchRequired = true)
        {
            const int srcRow = _sourceIndex.row();
            auto it = std::lower_bound(sourceRowMap_.begin(), sourceRowMap_.end(), srcRow);
            if (_matchRequired && !match(_sourceIndex))
            {
                if (it != sourceRowMap_.end() && *it == srcRow) // doesn't match any more
                    sourceRowMap_.erase(it);
                return -1;
            }

            if (it != sourceRowMap_.end() && *it == srcRow)
                return std::distance(sourceRowMap_.begin(), it);

            it = sourceRowMap_.insert(it, srcRow);
            return std::distance(sourceRowMap_.begin(), it);
        }

        bool contains(const QModelIndex& _sourceIndex) const
        {
            return std::binary_search(sourceRowMap_.begin(), sourceRowMap_.end(), _sourceIndex.row());
        }

        int rank(const QModelIndex& _sourceIndex) const
        {
            const int row = _sourceIndex.row();
            auto it = std::lower_bound(sourceRowMap_.cbegin(), sourceRowMap_.cend(), row);
            return (it != sourceRowMap_.cend() && *it == row) ? std::distance(sourceRowMap_.begin(), it) : -1;
        }

        int sourceRow(int _proxyRow) const
        {
            if (_proxyRow < 0 || _proxyRow >= childCount())
                return -1;
            return sourceRowMap_[_proxyRow];
        }

        void swapRows(std::deque<int>& _rows) noexcept
        {
            std::swap(_rows, sourceRowMap_);
        }

        void removeSourceRows(int _groupRow, int _first, int _last, QtGroupingProxyModel* _proxy)
        {
            if (sourceRowMap_.empty() || _first >= _last)
                return;

            if (_first > sourceRowMap_.back())
                return;

            const int d = _last - _first;
            auto lowerIt = std::lower_bound(sourceRowMap_.begin(), sourceRowMap_.end(), _first);
            auto upperIt = std::upper_bound(lowerIt, sourceRowMap_.end(), _last - 1);
            if (lowerIt != upperIt)
            {
                const QModelIndex parent = _proxy->index(_groupRow, 0);
                const int rowFirst = std::distance(sourceRowMap_.begin(), lowerIt);
                const int rowLast = std::distance(sourceRowMap_.begin(), upperIt);
                _proxy->beginRemoveRows(parent, rowFirst, std::max(0, rowLast - 1));
                sourceRowMap_.erase(lowerIt, upperIt);
                _proxy->endRemoveRows();
            }
            auto it = std::lower_bound(sourceRowMap_.begin(), sourceRowMap_.end(), _last - 1);
            std::transform(it, sourceRowMap_.end(), it, [d](int r) { return r - d; });
        }

    private:
        QVariant itemData(int _role, const QtGroupingProxyModel* _proxy) const
        {
            return (delegate_ ? delegate_->data(_role) : _proxy->ungroppedData(_role));
        }

    private:
        std::unique_ptr<QtGroupItemDataDelegate> delegate_;
        std::deque<int> sourceRowMap_;
        int checkedCount_ = 0;
        mutable Qt::ItemFlags cachedFlags_ = Qt::NoItemFlags;
    };

    enum NotificationType
    {
        NoNotifications = 0,
        NotifyGroupsChanged = 1 << 0,
        NotifyDataChanged = 1 << 1
    };

    using DataMap = std::unordered_map<int, QVariant>;
    using GroupMap = std::deque<GroupItem>;

    DataMap ungrouppedDataMap_;
    GroupMap groups_;
    QPointer<QAbstractItemModel> model_;
    QtGroupingProxyModel* q = nullptr;
    QtGroupingProxyModel::UngrouppedPolicy ungrouppedPolicy_ = QtGroupingProxyModel::UngrouppedAutoHide;
    QtGroupingProxyModel::UngrouppedPlacement ungrouppedPlacement_ = QtGroupingProxyModel::UngrouppedAtBack;
    int column_ = 0;
    bool spanGroups_ = true;
    bool identity_ = false;

    QtGroupingProxyModelPrivate(QtGroupingProxyModel* _model)
        : q(_model)
    {
    }

    QModelIndex groupIndex(GroupMap::const_iterator it) const
    {
        return it != groups_.end() ? q->index(std::distance(groups_.begin(), it), 0, {}) : QModelIndex{};
    }

    auto findGroup(const QString& _name) const
    {
        return std::find(groups_.begin(), groups_.end(), _name);
    }

    auto findGroup(const QtGroupItemDataDelegate* _delegate) const
    {
        return std::find(groups_.begin(), groups_.end(), _delegate);
    }

    auto findGroup(const QtGroupItemDataDelegate* _delegate)
    {
        return std::find(groups_.begin(), groups_.end(), _delegate);
    }

    int eraseGroup(const QString& name)
    {
        auto it = findGroup(name);
        if (it == groups_.end())
            return 0;

        groups_.erase(it);
        return 1;
    }

    auto eraseGroup(const QtGroupItemDataDelegate* _delegate)
    {
        auto it = findGroup(_delegate);
        if (it == groups_.end())
            return 0;

        groups_.erase(it);
        return 1;
    }

    void clearGroups()
    {
        for (auto& group : groups_)
        {
            group.clear();
            group.clearFlags();
        }
    }

    void resetGroups()
    {
        // block signals (like begin[Insert/Remove]Rows()) from
        // proxy model since resetGroups() method always called
        // in [begin/end]ResetModel() wrapped code
        QSignalBlocker blocker(q);
        clearGroups();

        // ensure that we remove previous uncategorized group if policy was changed
        if (ungrouppedPolicy_ == QtGroupingProxyModel::UngrouppedAlwaysOff)
            eraseGroup(nullptr);

        // Create uncategorized group if it must be
        // created and it isn't created already
        std::pair<GroupItem*, int> uncategorized = createUncategorizedGroup();

        // push indices into groups
        for (int row = 0, n = model_->rowCount(); row < n; ++row)
        {
            const QModelIndex index = model_->index(row, column_);
            if (!index.isValid())
                continue;

            int matchCount = 0;
            for (int i = 0, k = groups_.size(); i < k; ++i)
                matchCount += groups_[i].push(q, i, index);

            if (matchCount == 0 && uncategorized.first)
                uncategorized.first->push(q, uncategorized.second, index, false);
        }

        // remove uncategorized group according to policy
        if (uncategorized.first && uncategorized.first->isEmpty() &&
            ungrouppedPolicy_ == QtGroupingProxyModel::UngrouppedAutoHide)
            groups_.erase(groups_.begin() + uncategorized.second);
    }

    void removeGroup(const QtGroupItemDataDelegate* _delegate)
    {
        auto it = findGroup(_delegate);
        if (it == groups_.end())
            return;

        // move source indices from removed group into temporary storage
        std::deque<int> sourceRows;
        it->swapRows(sourceRows);

        const int removedRow = std::distance(groups_.begin(), it);
        q->beginRemoveRows({}, removedRow, removedRow);
        // erase group
        groups_.erase(it);
        q->endRemoveRows();

        // reassign source indices from removed group
        updateGroups(sourceRows.begin(), sourceRows.end(), NotifyGroupsChanged);
    }

    void insertGroup(QtGroupItemDataDelegate* _delegate)
    {
        auto insertionPoint = findGroup(nullptr);
        if (insertionPoint == groups_.begin())
            insertionPoint = groups_.end();

        const int insertedRow = std::distance(groups_.begin(), insertionPoint);
        q->beginInsertRows({}, insertedRow, insertedRow);
        // emplace new group
        groups_.emplace(insertionPoint, _delegate);
        q->endInsertRows();

        if (!model_)
            return;

        // Create uncategorized group if it must be
        // created and it isn't created already
        std::pair<GroupItem*, int> uncategorized = createUncategorizedGroup();

        // We have to find new group since insertion
        // can invalidate references/iterators
        auto groupIt = findGroup(_delegate);

        if (uncategorized.first)
            uncategorized.first->clear();

        // push indices into new group
        for (int row = 0, n = model_->rowCount(); row < n; ++row)
        {
            int matchCount = 0;
            const QModelIndex index = model_->index(row, column_, {});
            if (!index.isValid())
                continue;

            for (int i = 0, k = groups_.size(); i < k; ++i)
            {
                auto& group = groups_[i];
                if (std::addressof(group) == std::addressof(*groupIt))
                    matchCount += groupIt->push(q, i, index);
                else
                    matchCount += group.contains(index);
            }
            // update uncategorized group if
            // index doesn't belong to any group
            if (matchCount == 0 && uncategorized.first)
                uncategorized.first->push(q, uncategorized.second, index, false);
        }

        // remove uncategorized group according to policy
        if (uncategorized.first && uncategorized.first->isEmpty() &&
            ungrouppedPolicy_ == QtGroupingProxyModel::UngrouppedAutoHide)
        {
            q->beginRemoveRows({}, uncategorized.second, uncategorized.second);
            groups_.erase(groups_.begin() + uncategorized.second);
            q->endRemoveRows();
        }


        for (auto& group : groups_)
            group.clearFlags();
    }

    template<class _InIt>
    void updateGroups(_InIt first, _InIt last, int notifications, const QVector<int>& _roles = {})
    {
        if (groups_.empty() || first == last || !model_)
            return; // nothing to update

        const int nrows = model_->rowCount();

        // ensure that we remove previous uncategorized group if policy was changed
        if (ungrouppedPolicy_ == QtGroupingProxyModel::UngrouppedAlwaysOff)
            eraseGroup(nullptr);

        // Create uncategorized group if it must be
        // created and it isn't created already
        std::pair<GroupItem*, int> uncategorized = createUncategorizedGroup();
        const QModelIndex uncategorizedIndex = uncategorized.first ? q->index(uncategorized.second, 0) : QModelIndex{};

        std::vector<int> prevCntrs;
        if (notifications & NotifyGroupsChanged)
        {
            prevCntrs.reserve(groups_.size());
            for (const auto& group : groups_)
                prevCntrs.emplace_back(group.childCount());
        }

        // update groups from indices of source model
        for (; first != last; ++first)
        {
            int srcRow = *first;
            if (srcRow < 0 || srcRow >= nrows)
                continue;

            int matchCount = 0;
            const QModelIndex index = model_->index(srcRow, column_, {});
            if (!index.isValid())
                continue;

            for (auto it = groups_.begin(); it != groups_.end(); ++it)
            {
                const int row = it->update(index);
                const bool success = row > -1;
                if (success && (notifications & NotifyDataChanged))
                {
                    const QModelIndex parentIndex = groupIndex(it);
                    const QModelIndex proxyIndex = q->index(row, column_, parentIndex);
                    Q_EMIT q->dataChanged(proxyIndex, proxyIndex, _roles);
                }
                matchCount += success;
            }

            // update uncategorized group that we created earlier
            if (matchCount == 0 && uncategorized.first)
            {
                const int row = uncategorized.first->update(index, false);
                const bool success = row > -1;
                if (success && (notifications & NotifyDataChanged))
                {
                    const QModelIndex proxyIndex = q->index(row, column_, uncategorizedIndex);
                    Q_EMIT q->dataChanged(proxyIndex, proxyIndex, _roles);
                }
            }
        }

        // remove uncategorized group according to policy
        if (uncategorized.first && uncategorized.first->isEmpty() &&
            ungrouppedPolicy_ == QtGroupingProxyModel::UngrouppedAutoHide)
        {
            q->beginRemoveRows({}, uncategorized.second, uncategorized.second);
            groups_.erase(groups_.begin() + uncategorized.second);
            q->endRemoveRows();
            //adjust prevCntrs
            if (!prevCntrs.empty())
                prevCntrs.erase(prevCntrs.begin() + uncategorized.second);
        }

        for (auto& group : groups_)
            group.resetChecked(q);

        if (!prevCntrs.empty())
            notifyGroupItemsChanged(prevCntrs);
    }
    void notifyGroupItemsChanged(const std::vector<int>& prevCntrs)
    {
        Q_ASSERT(prevCntrs.size() == groups_.size());
        for (size_t i = 0, n = groups_.size(); i < n; ++i)
        {
            int prev = prevCntrs[i];
            int curr = groups_[i].childCount();
            if (curr == prev)
                continue;

            const QModelIndex index = q->index(i, 0, {});
            prev = std::clamp(prev, 0, q->rowCount(index) - 1);
            if (curr > prev)
            {
                q->beginInsertRows(index, prev, curr);
                q->endInsertRows();
            }
            else
            {
                q->beginRemoveRows(index, curr, prev);
                q->endRemoveRows();
            }
            Q_EMIT q->dataChanged(index, index);
        }
    }

    std::pair<GroupItem*, int> createUncategorizedGroup()
    {
        if (ungrouppedPolicy_ == QtGroupingProxyModel::UngrouppedAlwaysOff)
            return { nullptr, -1 };

        auto it = findGroup(nullptr);
        if (it != groups_.cend())
            return { std::addressof(*it), std::distance(groups_.begin(), it) };

        switch (ungrouppedPlacement_)
        {
        case QtGroupingProxyModel::UngrouppedAtFront:
            groups_.emplace_front(nullptr);
            return { std::addressof(groups_.front()), 0 };
        case QtGroupingProxyModel::UngrouppedAtBack:
            groups_.emplace_back(nullptr);
            return { std::addressof(groups_.back()), static_cast<int>(groups_.size() - 1) };
        default:
            break;
        }
        return { nullptr, -1 };
    }
};


QtGroupingProxyModel::QtGroupingProxyModel(QObject* _parent)
    : QIdentityProxyModel(_parent)
    , d(new QtGroupingProxyModelPrivate(this))
{
}

QtGroupingProxyModel::~QtGroupingProxyModel() = default;

void QtGroupingProxyModel::setSourceModel(QAbstractItemModel * _model)
{
    if (d->model_ == _model)
        return;

    if (d->model_)
    {
        disconnect(d->model_, &QAbstractItemModel::rowsInserted, this, &QtGroupingProxyModel::onSourceRowsInserted);
        disconnect(d->model_, &QAbstractItemModel::rowsRemoved, this, &QtGroupingProxyModel::onSourceRowsRemoved);
        disconnect(d->model_, &QAbstractItemModel::rowsMoved, this, &QtGroupingProxyModel::onSourceRowsMoved);
        disconnect(d->model_, &QAbstractItemModel::dataChanged, this, &QtGroupingProxyModel::onSourceDataChanged);
        disconnect(d->model_, &QAbstractItemModel::destroyed, this, &QtGroupingProxyModel::onSourceDestroyed);
        disconnect(d->model_, &QAbstractItemModel::modelReset, this, &QtGroupingProxyModel::onSourceReset);
        disconnect(d->model_, &QAbstractItemModel::headerDataChanged, this, &QtGroupingProxyModel::headerDataChanged);
    }

    d->model_ = _model;
    if (d->model_)
    {
        connect(d->model_, &QAbstractItemModel::rowsInserted, this, &QtGroupingProxyModel::onSourceRowsInserted);
        connect(d->model_, &QAbstractItemModel::rowsRemoved, this, &QtGroupingProxyModel::onSourceRowsRemoved);
        connect(d->model_, &QAbstractItemModel::rowsMoved, this, &QtGroupingProxyModel::onSourceRowsMoved);
        connect(d->model_, &QAbstractItemModel::dataChanged, this, &QtGroupingProxyModel::onSourceDataChanged);
        connect(d->model_, &QAbstractItemModel::destroyed, this, &QtGroupingProxyModel::onSourceDestroyed);
        connect(d->model_, &QAbstractItemModel::modelReset, this, &QtGroupingProxyModel::onSourceReset);
        connect(d->model_, &QAbstractItemModel::headerDataChanged, this, &QtGroupingProxyModel::headerDataChanged);
    }

    QAbstractProxyModel::setSourceModel(_model);
    onSourceReset(); // update our groups
}

void QtGroupingProxyModel::setIdentity(bool _on)
{
    if (d->identity_ == _on)
        return;

    d->identity_ = _on;
    onSourceReset();
    Q_EMIT identityChanged(_on);
}

bool QtGroupingProxyModel::isIdentity() const
{
    return d->identity_;
}

void QtGroupingProxyModel::setGroupColumn(int _column)
{
    if (d->column_ == _column)
        return;

    d->column_ = _column;
    onSourceReset();
    Q_EMIT groupColumnChanged(_column);
}

int QtGroupingProxyModel::groupColumn() const
{
    return d->column_;
}

void QtGroupingProxyModel::setGroupsSpanned(bool _on)
{
    if (d->spanGroups_ == _on)
        return;

    Q_EMIT layoutAboutToBeChanged();
    d->spanGroups_ = _on;
    Q_EMIT layoutChanged();
    Q_EMIT groupsSpannedChanged(_on);
}

bool QtGroupingProxyModel::isGroupsSpanned() const
{
    return d->spanGroups_;
}

void QtGroupingProxyModel::setUngrouppedPolicy(QtGroupingProxyModel::UngrouppedPolicy _policy)
{
    if (d->ungrouppedPolicy_ == _policy)
        return;

    d->ungrouppedPolicy_ = _policy;
    beginResetModel();
    d->clearGroups();
    if (d->model_)
        d->resetGroups();
    endResetModel();
}

QtGroupingProxyModel::UngrouppedPolicy QtGroupingProxyModel::ungrouppedPolicy() const
{
    return d->ungrouppedPolicy_;
}

void QtGroupingProxyModel::setUngrouppedPlacement(QtGroupingProxyModel::UngrouppedPlacement _placement)
{
    if (d->ungrouppedPlacement_ == _placement)
        return;

    d->ungrouppedPlacement_ = _placement;
    d->eraseGroup(nullptr);

    beginResetModel();
    d->clearGroups();
    if (d->model_)
        d->resetGroups();
    endResetModel();
}

QtGroupingProxyModel::UngrouppedPlacement QtGroupingProxyModel::ungrouppedPlacement() const
{
    return d->ungrouppedPlacement_;
}

QtGroupItemDataDelegate* QtGroupingProxyModel::group(const QString & _groupName, QtAbstractItemFilter *_filter)
{
    if (!_filter)
        return nullptr;

    return group(new FilterGroupItemDelegate(_groupName, _filter));
}

QtGroupItemDataDelegate* QtGroupingProxyModel::group(QtGroupItemDataDelegate * _delegate)
{
    if (!_delegate)
        return nullptr;

    auto it = d->findGroup(_delegate);
    if (it != d->groups_.end())
        return _delegate; // same delegate

    d->insertGroup(_delegate);
    return _delegate;
}

void QtGroupingProxyModel::ungroup(const QString & name)
{
    auto it = d->findGroup(name);
    if (it == d->groups_.end())
        return;
    ungroup(it->delegate());
}

void QtGroupingProxyModel::ungroup(const QtGroupItemDataDelegate * _delegate)
{
    if (!_delegate)
        return;

    d->removeGroup(_delegate);
}

void QtGroupingProxyModel::ungroup()
{
    d->groups_.clear();
    if (!d->model_)
        return;

    // full update
    refresh();
}

QModelIndex QtGroupingProxyModel::groupIndex(const QtGroupItemDataDelegate * _delegate) const
{
    return d->identity_ ? QModelIndex{} : d->groupIndex(d->findGroup(_delegate));
}

QModelIndex QtGroupingProxyModel::groupIndex(const QString & _name) const
{
    return d->identity_ ? QModelIndex{} : d->groupIndex(d->findGroup(_name));
}

QVariant QtGroupingProxyModel::ungroppedData(int _role) const
{
    auto it = d->ungrouppedDataMap_.find(_role);
    return it != d->ungrouppedDataMap_.cend() ? it->second : QVariant{};
}

bool QtGroupingProxyModel::setUngrouppedData(const QVariant & _value, int _role)
{
    if (!_value.isValid())
    {
        d->ungrouppedDataMap_.erase(_role);
        return false;
    }
    else
    {
        d->ungrouppedDataMap_[_role] = _value;
        return true;
    }
}

QModelIndex QtGroupingProxyModel::index(int _row, int _column, const QModelIndex & _parent) const
{
    if (d->identity_)
        return QIdentityProxyModel::index(_row, _column, _parent);

    // Here is a tricky part:
    // - if we have a parent set internal id to it row
    // - otherwise set internal id to -1 sentinel
    // It's important that the -1 sentinel allows to
    // distinguish child/parent items
    if (_parent.isValid())
        return createIndex(_row, _column, quintptr(_parent.row()));
    else
        return createIndex(_row, _column, quintptr(-1));
}

QModelIndex QtGroupingProxyModel::parent(const QModelIndex & _child) const
{
    if (d->identity_)
        return QIdentityProxyModel::parent(_child);

    if (!_child.isValid())
        return QModelIndex();

    // Here is a tricky part:
    // - if internal id equals -1 then the child is top level item: return the invalid model index
    // - if internal id is something other than -1 than the child is some kind of child item so we
    // need to set column of parent to 0 when creating the parent index, to escape selection issues

    if (_child.internalId() == quintptr(-1))
        return QModelIndex{};
    else
        return createIndex(_child.internalId(), 0, quintptr(-1));
}

int QtGroupingProxyModel::rowCount(const QModelIndex & _parent) const
{
    if (d->identity_)
        return QIdentityProxyModel::rowCount(_parent);

    if (_parent.isValid())
        return (_parent.internalId() == quintptr(-1) ? d->groups_[_parent.row()].childCount() : 0);
    else
        return static_cast<int>(d->groups_.size());
}

int QtGroupingProxyModel::columnCount(const QModelIndex & _parent) const
{
    if (d->identity_)
        return QIdentityProxyModel::columnCount(_parent);

    // be aware of setting Qt::ItemNeverHasChildren in
    // source model leads to incorrect behavior
    return d->model_ ? d->model_->columnCount() : 0;
}

QVariant QtGroupingProxyModel::data(const QModelIndex & proxyIndex, int role) const
{
    if (!proxyIndex.isValid() || !d->model_)
        return {};

    if (d->identity_)
        return QIdentityProxyModel::data(proxyIndex, role);

    const int id = proxyIndex.internalId();
    const int r = proxyIndex.row();
    const int c = proxyIndex.column();
    if (id == -1) // top-level item
    {
        if (c != d->column_ || r < 0 || r >= static_cast<int>(d->groups_.size()))
            return {};
        else
            return d->groups_[r].data(role, this);
    }

    const QModelIndex parent = proxyIndex.parent();
    if (!parent.isValid())
        return {};

    const int row = d->groups_[parent.row()].sourceRow(r);
    if (row == -1)
        return {};
    else
        return d->model_->index(row, c).data(role);
}

bool QtGroupingProxyModel::setData(const QModelIndex & proxyIndex, const QVariant & value, int role)
{
    if (!proxyIndex.isValid() || !d->model_)
        return false;

    if (d->identity_)
        return QIdentityProxyModel::setData(proxyIndex, value, role);

    const int id = proxyIndex.internalId();
    const int r = proxyIndex.row();
    const int c = proxyIndex.column();
    if (id == -1) // top-level item
    {
        if (c != d->column_ || r < 0 || r >= static_cast<int>(d->groups_.size()))
            return false;

        if (d->groups_[r].setData(value, role, this))
            Q_EMIT dataChanged(proxyIndex, proxyIndex, QVector<int>() << role);
    }
    const QModelIndex parent = proxyIndex.parent();
    if (!parent.isValid())
        return false;

    const int row = d->groups_[parent.row()].sourceRow(r);
    if (row == -1)
        return false;

    return d->model_->setData(d->model_->index(row, c), value, role);
}

Qt::ItemFlags QtGroupingProxyModel::flags(const QModelIndex & _proxyIndex) const
{
    if (!_proxyIndex.isValid() || !d->model_)
        return Qt::NoItemFlags;

    if (d->identity_)
        return QIdentityProxyModel::flags(_proxyIndex);

    const int id = _proxyIndex.internalId();
    const int r = _proxyIndex.row();
    const int c = _proxyIndex.column();
    if (id == -1) // top-level item
    {
        if (c != d->column_ || r < 0 || r >= static_cast<int>(d->groups_.size()))
            return Qt::NoItemFlags;

        return d->groups_[r].flags(this);
    }

    const QModelIndex parent = _proxyIndex.parent();
    if (!parent.isValid())
        return Qt::ItemIsEnabled;

    const int row = d->groups_[parent.row()].sourceRow(r);
    if (row == -1)
        return Qt::NoItemFlags;

    return d->model_->index(row, c).flags();
}

QVariant QtGroupingProxyModel::headerData(int _section, Qt::Orientation _orientation, int _role) const
{
    if (d->model_)
        return d->model_->headerData(_section, _orientation, _role);
    else
        return QAbstractItemModel::headerData(_section, _orientation, _role);
}

bool QtGroupingProxyModel::setHeaderData(int _section, Qt::Orientation _orientation, const QVariant & _value, int _role)
{
    if (d->model_)
        return d->model_->setHeaderData(_section, _orientation, _value, _role);
    else
        return QAbstractItemModel::setHeaderData(_section, _orientation, _value, _role);
}

void QtGroupingProxyModel::refresh()
{
    onSourceReset();
}

void QtGroupingProxyModel::onSourceRowsInserted(const QModelIndex & _parent, int _first, int _last)
{
    using NotificationType = QtGroupingProxyModelPrivate::NotificationType;
    if (d->identity_)
    {
        beginInsertRows(mapFromSource(_parent), _first, _last);
        endInsertRows();
        return;
    }

    d->updateGroups(CountingIterator(_first),
                    CountingIterator(d->model_->rowCount()),
                    NotificationType::NotifyGroupsChanged | NotificationType::NotifyDataChanged);
}

void QtGroupingProxyModel::onSourceRowsRemoved(const QModelIndex & _parent, int _first, int _last)
{
    if (d->identity_)
    {
        beginRemoveRows(mapFromSource(_parent), _first, _last);
        endInsertRows();
        return;
    }

    for (auto it = d->groups_.begin(), end = d->groups_.end(); it != end; ++it)
    {
        const int i = std::distance(d->groups_.begin(), it);
        it->removeSourceRows(i, _first, _last + 1, this);
    }
}

void QtGroupingProxyModel::onSourceRowsMoved(const QModelIndex & _parent, int _start, int _end, const QModelIndex & _dest, int _row)
{
    using NotificationType = QtGroupingProxyModelPrivate::NotificationType;

    if (d->identity_)
    {
        beginMoveRows(mapFromSource(_parent), _start, _end, mapFromSource(_dest), _row);
        endInsertRows();
        return;
    }

    // we have to update indices in our groups
    d->updateGroups(CountingIterator(0),
                    CountingIterator(d->model_->rowCount()),
                    NotificationType::NotifyGroupsChanged | NotificationType::NotifyDataChanged);
}

void QtGroupingProxyModel::onSourceDataChanged(const QModelIndex & _topLeft, const QModelIndex & _bottomRight, const QVector<int>&_roles)
{
    using NotificationType = QtGroupingProxyModelPrivate::NotificationType;

    if (d->identity_)
    {
        Q_EMIT dataChanged(mapFromSource(_topLeft), mapFromSource(_bottomRight), _roles);
        return;
    }
    d->updateGroups(CountingIterator(_topLeft.row()),
                    CountingIterator(_bottomRight.row() + 1),
                    NotificationType::NotifyGroupsChanged | NotificationType::NotifyDataChanged,
                    _roles);
}

void QtGroupingProxyModel::onSourceReset()
{
    d->clearGroups();
    if (!d->model_)
        return;

    beginResetModel();
    d->resetGroups();
    endResetModel();
}

void QtGroupingProxyModel::onSourceDestroyed()
{
    d->clearGroups();
}

QModelIndex QtGroupingProxyModel::buddy(const QModelIndex& _index) const
{
    if (d->identity_)
        return QIdentityProxyModel::buddy(_index);
    else
        return _index;
}

QModelIndex QtGroupingProxyModel::mapToSource(const QModelIndex & _proxyIndex) const
{
    if (d->identity_)
        return QIdentityProxyModel::mapToSource(_proxyIndex);

    const int id = _proxyIndex.internalId();
    const int r = _proxyIndex.row();
    const int c = _proxyIndex.column();
    if (id == -1) // top-level item
        return QModelIndex{};

    const QModelIndex parent = _proxyIndex.parent();
    if (!parent.isValid() || parent.row() < 0 || parent.row() >= static_cast<int>(d->groups_.size()))
        return QModelIndex{};

    const int row = d->groups_[parent.row()].sourceRow(r);
    if (row == -1)
        return QModelIndex{};

    return d->model_->index(row, c);
}

QModelIndex QtGroupingProxyModel::mapFromSource(const QModelIndex & _sourceIndex) const
{
    if (d->identity_)
        return QIdentityProxyModel::mapFromSource(_sourceIndex);

    for (auto it = d->groups_.cbegin(); it != d->groups_.cend(); ++it)
    {
        const int r = it->rank(_sourceIndex);
        if (r != -1)
        {
            const QModelIndex parentIndex = this->index(std::distance(d->groups_.cbegin(), it), 0);
            return this->index(r, 0, parentIndex);
        }
    }
    return QModelIndex{};
}
