#pragma once
#include <utility>
#include <algorithm>

#include <QtWidgetsExtra>

class QAbstractItemModel;
class QModelIndex;

class QTWIDGETSEXTRA_EXPORT QtItemModelSegregator
{
    template <class _Container>
    struct inserter
    {
        using iterator = typename _Container::iterator;

        _Container& storage;

        explicit inserter(_Container& c)
            : storage(c)
        {
        }

        template <class _InIt>
        iterator operator()(iterator pos, _InIt first, _InIt last, iterator& e)
        {
            pos = storage.insert(pos, first, last);
            e = storage.end();
            return pos;
        }
    };

    template <class _Container>
    struct eraser
    {
        using iterator = typename _Container::iterator;

        _Container& storage;

        explicit eraser(_Container& c)
            : storage(c)
        {
        }

        iterator operator()(iterator first, iterator last, iterator& e)
        {
            last = storage.erase(first, last);
            e = storage.end();
            return last;
        }
    };

public:
    virtual ~QtItemModelSegregator() = default;

    template<
            class _Container1,
            class _Container2
            >
    void segregate(_Container1& prev, _Container2& curr,
                   QAbstractItemModel* m,
                   int column = 0,
                   const QModelIndex& parent = {})
    {
        this->segregate(prev, curr, std::less<>{}, std::equal_to{}, m, column, parent);
    }

    template<
        class _Container1,
        class _Container2,
        class _KeyComp,
        class _ValueEq
    >
    void segregate(_Container1& prev, _Container2& curr,
                   _KeyComp comp, _ValueEq eq,
                   QAbstractItemModel* m,
                   int column = 0,
                   const QModelIndex& parent = {})
    {
        this->segregate(std::begin(prev), std::end(prev),
                        std::begin(curr), std::end(curr),
                        inserter<_Container1>(prev),
                        eraser<_Container1>(prev),
                        comp, eq, m, column, parent);
    }

private:
    template<
        class _InIt1,
        class _InIt2,
        class _Inserter,
        class _Eraser,
        class _KeyComp,
        class _ValueEq
    >
    void segregate(_InIt1 first1, _InIt1 last1,
                   _InIt2 first2, _InIt2 last2,
                   _Inserter inserter, _Eraser eraser,
                   _KeyComp comp, _ValueEq eq,
                   QAbstractItemModel* m,
                   int column = 0,
                   const QModelIndex& parent = {})
    {
        if (first2 == last2)
        {
            if (first1 == last1)
                return;
            beginReset(m);
            eraser(first1, last1, last1);
            endReset(m);
            return;
        }

        if (first1 == last1)
        {
            if (first2 == last2)
                return;
            beginReset(m);
            inserter(last1, first2, last2, last1);
            endReset(m);
            return;
        }

        int row = 0;
        while (first1 != last1 && first2 != last2)
        {
            const auto end1 = advance_to(first1, last1, *first2, comp); // prev < curr
            if (end1.first != first1)
            {
                beginRemove(m, parent, row, row + end1.second - 1);
                first1 = eraser(first1, end1.first, last1);
                endRemove(m);
                continue;
            }

            const auto end2 = advance_to(first2, last2, *first1, comp); // prev > curr
            if (end2.first != first2)
            {
                beginInsert(m, parent, row, row + end2.second - 1);
                first1 = inserter(first1, first2, end2.first, last1);
                row += end2.second;
                endInsert(m);
                continue;
            }

            // prev == curr
            if (!eq(*first1, *first2))
            {
                *first1 = std::move(*first2);
                notifyChanged(m, parent, row, column);
            }
            ++first1;
            ++first2;
            ++row;
        }

        if (first1 != last1)
        {
            const auto n = std::distance(first1, last1);
            beginRemove(m, parent, row, row + n - 1);
            eraser(first1, last1, last1);
            endRemove(m);
        }

        if (first2 != last2)
        {
            const auto n = std::distance(first2, last2);
            beginInsert(m, parent, row, row + n - 1);
            inserter(first1, first2, last2, last1);
            endInsert(m);
        }
    }

    template<class _It, class _Value, class _Comp>
    static std::pair<_It, std::ptrdiff_t> advance_to(_It first, _It last, const _Value& value, _Comp comp)
    {
        std::ptrdiff_t n = 0;
        if (first == last)
            return { first, n };
        while (comp(*first, value))
        {
            ++first;
            if (first == last)
                break;
            ++n;
        }
        return { first, n };
    }

protected:
    virtual void beginReset(QAbstractItemModel* m);
    virtual void endReset(QAbstractItemModel* m);

    virtual void beginInsert(QAbstractItemModel* m, const QModelIndex& parent, int first, int last);
    virtual void endInsert(QAbstractItemModel* m);

    virtual void beginRemove(QAbstractItemModel* m, const QModelIndex& parent, int first, int last);
    virtual void endRemove(QAbstractItemModel* m);

    virtual void notifyChanged(QAbstractItemModel* m, const QModelIndex& parent, int row, int column, std::initializer_list<int> roles = {});
};
