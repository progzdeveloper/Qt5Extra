#pragma once
#include <utility>
#include <functional>
#include <iterator>
#include <QVarLengthArray>
#include "stdhash_support.h"

namespace Qt5Extra
{
    template<
        class _Key,
        class _Value,
        int _Prealloc = 16,
        class _Hash = std::hash<_Key>,
        class _KeyEq = std::equal_to<_Key>
    >
    class FlatMap : public _Hash, public _KeyEq
    {
    public:
        static constexpr int kPreallocSize = _Prealloc;
        static constexpr int kMaxSize = 256;

        static_assert (_Prealloc >= 0, "preallocation size can't be negative");
        static_assert (_Prealloc <= kMaxSize, "preallocation size is too big");

        using key_type = _Key;
        using mapped_type = _Value;
        using value_type = std::pair<_Key, _Value>;
        using reference = value_type&;
        using const_reference = const value_type&;
        using pointer = value_type*;
        using const_pointer = const value_type*;
        using storage_type = QVarLengthArray<value_type, kPreallocSize>;
        using lookup_type = QVarLengthArray<size_t, kPreallocSize>;

        using iterator = typename storage_type::iterator;
        using const_iterator = typename storage_type::const_iterator;

        FlatMap() {}

        template<class _It>
        FlatMap(_It first, _It last)
        {
            this->insert(first, last);
        }

        iterator move_front(iterator it)
        {
            if (!empty())
                std::iter_swap(storage.begin(), it);
            return storage.begin();
        }

        int move_front(const key_type& key)
        {
            auto it = find(key);
            if (it == storage.end())
                return 0;

            move_front(iterator{ it });
            return 1;
        }

        template<class _It>
        void insert(_It first, _It last)
        {
            using iter_category = typename std::iterator_traits<_It>::category;
            if constexpr (std::is_base_of<iter_category, std::random_access_iterator_tag>::value)
                storage.reserve(storage.size() + std::distance(first, last));

            for (; first != last; ++first)
                insert(*first);
        }

        std::pair<bool, iterator> insert(const value_type& v)
        {
            if (storage.size() >= kMaxSize)
                throw std::overflow_error("FlatMap overflow");

            auto it = find(v.first);
            if (it != storage.end())
                return { false, it };

            lookup.push_back(key_hash(v.first));
            storage.push_back(v);
            it = storage.end();
            --it;

            return { true, it };
        }

        template<class... _Args>
        auto emplace(_Args&&... args)
        {
            value_type v(std::forward<_Args>(args)...);
            return insert(v);
        }

        size_t erase(const key_type& k)
        {
            auto it = find(k);
            if (it == storage.end())
                return 0;

            const auto d = std::distance(storage.begin(), it);
            std::swap(lookup[d], lookup.back());
            std::swap(*it, storage.back());
            storage.pop_back();
            lookup.pop_back();
            return 1;
        }

        void clear() noexcept { storage.clear(); }

        void reserve(int n) { storage.reserve(std::min(n, kMaxSize)); }

        mapped_type& operator[](const key_type& key)
        {
            auto it = find(key);
            if (it != storage.end())
                return const_cast<mapped_type&>(it->second);

            insert({ key, {} });
            return storage.back().second;
        }

        template<class _TKey>
        const mapped_type& value(const _TKey& key, const mapped_type& fallback = {}) const
        {
            auto it = find(key);
            return (it != storage.end() ? it->second : fallback);
        }

        const_reference front() const { return storage.front(); }
        const_reference back() const { return storage.back(); }

        template<class _TKey>
        const_iterator find(const _TKey& k) const
        {
            const size_t h = key_hash(static_cast<const _Key&>(k));
            auto lIt = lookup.begin();
            for(;;)
            {
                lIt = std::find(lIt, lookup.end(), h);
                if (lIt == lookup.end())
                    break;

                auto it = storage.begin() + std::distance(lookup.begin(), lIt);
                if (equal(it->first, k))
                    return it;
            }
            return storage.end();
        }

        template<class _TKey>
        iterator find(const _TKey& k)
        {
            const size_t h = key_hash(static_cast<const _Key&>(k));
            auto lIt = lookup.begin();
            for(;;)
            {
                lIt = std::find(lIt, lookup.end(), h);
                if (lIt == lookup.end())
                    break;

                auto it = storage.begin() + std::distance(lookup.begin(), lIt);
                if (equal(it->first, k))
                    return it;
            }
            return storage.end();
        }

        iterator begin() { return storage.begin(); }
        iterator end() { return storage.end(); }

        const_iterator begin() const { return storage.begin(); }
        const_iterator end() const { return storage.end(); }

        const_iterator cbegin() const { return storage.begin(); }
        const_iterator cend() const { return storage.end(); }

        size_t size() const noexcept { return storage.size(); }
        size_t empty() const noexcept { return storage.empty(); }

        template<class _TKey>
        bool contains(const _TKey& key) const noexcept
        {
            return find(key) != storage.end();
        }

        void merge(const FlatMap& other)
        {
            for (auto it = other.cbegin(); it != other.cend(); ++it)
            {
                if (contains(it->first))
                    continue;

                (*this)[it->first] = it->second;
            }
        }

    private:
        template<class _LKey, class _RKey>
        bool equal(const _LKey& lhs, const _RKey& rhs) const noexcept
        {
            return static_cast<const _KeyEq&>(*this)(lhs, rhs);
        }

        size_t key_hash(const key_type& k) const noexcept
        {
            return static_cast<const _Hash&>(*this)(k);
        }

    private:
        storage_type storage;
        lookup_type lookup;
    };
} // end namespace Qt5Extra


