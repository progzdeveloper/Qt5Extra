#pragma once
#include <algorithm>
#include <iterator>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <set>

#ifdef QT_VERSION
#include <QHash>
#include <QMap>
#include <QSet>
#endif

namespace Qt5Extra
{
    template<class _It, class T>
    constexpr bool contains(_It first, _It last, const T& value)
    {
        for (; first != last; ++first)
            if (*first == value)
                return true;
        return false;
    }

    template<class _Container, class T>
    constexpr bool contains(const _Container& c, const T& value)
    {
        return contains(std::begin(c), std::end(c), value);
    }

    // sets and maps
    template<class _Key, class _Value, class _Comp, class _Al>
    static inline bool contains(const std::map<_Key, _Value, _Comp, _Al>& c, const _Key& key)
    {
        return c.count(key) > 0;
    }

    template<class _Key, class _Value, class _Comp, class _Al>
    static inline bool contains(const std::multimap<_Key, _Value, _Comp, _Al>& c, const _Key& key)
    {
        return c.count(key) > 0;
    }

    template<class _Key, class _Comp, class _Al>
    static inline bool contains(const std::set<_Key, _Comp, _Al>& c, const _Key& key)
    {
        return c.count(key) > 0;
    }

    template<class _Key, class _Comp, class _Al>
    static inline bool contains(const std::multiset<_Key, _Comp, _Al>& c, const _Key& key)
    {
        return c.count(key) > 0;
    }

    // unordered sets and maps
    template<class _Key, class _Value, class _Hash, class _Pr, class _Al>
    static inline bool contains(const std::unordered_map<_Key, _Value, _Hash, _Pr, _Al>& c, const _Key& key)
    {
        return c.count(key) > 0;
    }

    template<class _Key, class _Value, class _Hash, class _Pr, class _Al>
    static inline bool contains(const std::unordered_multimap<_Key, _Value, _Hash, _Pr, _Al>& c, const _Key& key)
    {
        return c.count(key) > 0;
    }

    template<class _Key, class _Hash, class _Pr, class _Al>
    static inline bool contains(const std::unordered_set<_Key, _Hash, _Pr, _Al>& c, const _Key& key)
    {
        return c.count(key) > 0;
    }

    template<class _Key, class _Hash, class _Pr, class _Al>
    static inline bool contains(const std::unordered_multiset<_Key, _Hash, _Pr, _Al>& c, const _Key& key)
    {
        return c.count(key) > 0;
    }

#ifdef QT_VERSION
    template<class _Key, class _Value>
    static inline bool contains(const QHash<_Key, _Value>& c, const _Key& key)
    {
        return c.contains(key);
    }

    template<class _Key>
    static inline bool contains(const QSet<_Key>& c, const _Key& key)
    {
        return c.contains(key);
    }

    template<class _Key, class _Value>
    static inline bool contains(const QMap<_Key, _Value>& c, const _Key& key)
    {
        return c.contains(key);
    }
#endif
} // end namespace Qt5Extra
