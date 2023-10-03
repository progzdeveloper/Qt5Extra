#pragma once
#include <algorithm>
#include <iterator>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <set>

namespace AlgExt
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
template<class _Key, class _Value>
static inline bool contains(const std::map<_Key, _Value>& c, const _Key& key)
{
    return c.count(key) > 0;
}

template<class _Key, class _Value>
static inline bool contains(const std::multimap<_Key, _Value>& c, const _Key& key)
{
    return c.count(key) > 0;
}

template<class _Key, class _Value>
static inline bool contains(const std::set<_Key, _Value>& c, const _Key& key)
{
    return c.count(key) > 0;
}

template<class _Key, class _Value>
static inline bool contains(const std::multiset<_Key, _Value>& c, const _Key& key)
{
    return c.count(key) > 0;
}

// unordered sets and maps
template<class _Key, class _Value>
static inline bool contains(const std::unordered_map<_Key, _Value>& c, const _Key& key)
{
    return c.count(key) > 0;
}

template<class _Key, class _Value>
static inline bool contains(const std::unordered_multimap<_Key, _Value>& c, const _Key& key)
{
    return c.count(key) > 0;
}

template<class _Key, class _Value>
static inline bool contains(const std::unordered_set<_Key, _Value>& c, const _Key& key)
{
    return c.count(key) > 0;
}

template<class _Key, class _Value>
static inline bool contains(const std::unordered_multiset<_Key, _Value>& c, const _Key& key)
{
    return c.count(key) > 0;
}

}
