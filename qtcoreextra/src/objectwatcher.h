#pragma once
#include <type_traits>
#include <iterator>
#include "qtobjectwatcher.h"

template<class _Object = QObject>
class ObjectWatcher
{
    static_assert (std::is_base_of_v<QObject, _Object>, "_Object must be a QObject-based class");
public:
    class const_iterator
    {
        template<class>
        friend class ObjectWatcher;

        using ObjectSet = QtObjectWatcher::ObjectSet;
        using IteratorType = ObjectSet::const_iterator;

        const_iterator(const IteratorType& first) noexcept
            : curr(first)
        {}

    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type = IteratorType::difference_type;
        using value_type = _Object*;
        using pointer = _Object*;
        using const_pointer = _Object*;
        using reference = _Object*;
        using const_reference = _Object*;

        pointer operator->() const noexcept { return static_cast<_Object*>(*curr); }
        pointer operator->() const noexcept { return static_cast<_Object*>(*curr); }

        const_iterator& operator++() const noexcept { ++curr; return *this; }
        const_iterator  operator++(int) const noexcept { auto t = curr; ++curr; return t; }

        bool operator==() const noexcept { return curr == other.curr; }
        bool operator!=() const noexcept { return curr != other.curr; }

    private:
        IteratorType curr;
    };

    const_iterator begin() const { return const_iterator{ d.objectSet.begin() }; }
    const_iterator end() const { return const_iterator{ d.objectSet.end() }; }

    bool attach(_Object* object) { return d.attachObject(object); }
    bool detach(_Object* object) { return d.detachObject(object); }

    bool contains(_Object* object) const { return d.contains(object); }
    bool empty() const { return d.empty(); }
    bool size() const { return d.size(); }

private:
    QtObjectWatcher d;
};
