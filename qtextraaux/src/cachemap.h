#pragma once
#include "cacheline.h"
#include "lrucache.h"

namespace Qt5Extra
{
    /*!
     * \brief The CacheMap<> class provide the ability to cache different
     * states of heavy objects in efficent way.
     * \tparam _Key type of primary part of the key
     * \tparam _Hint type of variable part of the key
     * \tparam _Value type of stored object
     *
     * \note this class is reenterant
     */
    template<
        class _Key,
        class _Hint,
        class _Value
    >
    class CacheMap
    {
        using cache_line = CacheLine<_Hint, _Value>;
        using lookup_type = LRUCache<_Key, cache_line>;

    public:
        using key_type = _Key;
        using hint_type = _Hint;
        using value_type = _Value;

        /* The copy-construction of cache_map is prohibited
         * since it's useless and we definetly don't want
         * to copy heavy objects inside cache storage
         */
        CacheMap(const CacheMap&) = delete;
        CacheMap& operator=(const CacheMap&) = delete;

        /* The move construction and assignment
         * is NOT prohibited, since it's cheap
         */
        CacheMap(CacheMap&&) = default;
        CacheMap& operator=(CacheMap&&) = default;

        /*!
         * \brief Default-construct the cache map with
         * unlimited size of primary and secondary cache.
         */
        CacheMap() = default;

        /*!
         * \brief Construct the cache map with provided limits.
         *
         * \param size size of primary keys cache lookup
         * \param depth size of secondary keys cache (hints)
         */
        explicit CacheMap(size_t size, size_t depth)
            : cache_(size)
            , depth_(depth)
        {
        }

        /*!
         * \brief Find the entity for specified primary and secondary keys.
         *
         * \param key primary key
         * \param hint secondary key (hint)
         * \param result extracted value if search was successfull
         * \return true if element was found, otherwise return false
         */
        bool find(const key_type& key, const hint_type& hint, value_type& result) const
        {
            auto it = cache_.find(key);
            if (it == cache_.end())
                return false;

            return it->second.find(hint, result);
        }

        /*!
         * \brief Find the entity for specified primary key using
         * the custom unary predicate for secondary key searching.
         *
         * \detail This method is useful when we want to find item
         * that falls into some range of hint values (for example
         * less than sertan picture size).
         * \note The specified predicate must satisfy the unary search predicate
         * conditions. That means, that the expression pred(v) must be convertible
         * to bool for every argument v of type (possibly const) and must not modify v.
         * \tparam type of unary predicate
         * \param pred unary predicate which returns true for the required hint
         * \param key primary key
         * \param result extracted value if search was successfull
         * \return true if element was found, otherwise return false
         */
        template<class _Pred>
        bool find_if(_Pred pred, const key_type& key, value_type& result) const
        {
            auto it = cache_.find(key);
            if (it == cache_.end())
                return false;

            return it->second.find_if(pred, result);
        }

        /*!
        * \brief Insert by coping a new entity into the cache.
        *
        * \param key primary key
        * \param hint secondary key (hint)
        * \param value value of entity
        */
        void insert(const key_type& key, const hint_type& hint, const value_type& value)
        {
            auto result = cache_.emplace(key, cache_line{ depth_ });
            result.first->second.emplace(hint, value);
        }

        /*!
         * \brief Insert by moving a new entity into the cache.
         *
         * \param key primary key
         * \param hint secondary key (hint)
         * \param value value of entity
         */
        void insert(const key_type& key, const hint_type& hint, value_type&& value)
        {
            auto result = cache_.emplace(key, cache_line{ depth_ });
            result.first->second.emplace(hint, std::forward<value_type>(value));
        }

        /*!
         * \brief Insert by creating a new entity in-place.
         *
         * \param key primary key
         * \param hint secondary key (hint)
         * \param value value of entity
         */
        template<class... _Args>
        void emplace(const key_type& key, const hint_type& hint, _Args&&... args)
        {
            auto result = cache_.emplace(key, cache_line{ depth_ });
            result.first->second.emplace(hint, std::forward<_Args>(args)...);
        }

        /*!
         * \brief Move entity with primary key to front
         * \param key key to move
         * \return number of items moved (0 or 1)
         */
        size_t move_front(const key_type& key)
        {
            return cache_.move_font(key);
        }

        /*!
         * \brief Erase the single entity from the cache.
         *
         * \param key primary key
         * \param hint secondary key (hint)
         * \return number of elements removed
         */
        size_t erase(const key_type& key, const hint_type& hint)
        {
            auto it = cache_.find(key);
            if (it == cache_.end())
                return 0;

            const size_t n = it->second.erase(hint);
            if (it->second.empty())
                cache_.erase(it);
            return n;
        }

        /*!
         * \brief Erase all entities for specified primary key.
         *
         * \param key primary key
         * \return number of elements removed
         */
        size_t erase(const key_type& key)
        {
            auto it = cache_.find(key);
            if (it == cache_.end())
                return 0;

            const size_t n = it->second.size();
            cache_.erase(it);
            return n;
        }

        /*!
         * \brief Clear the cache.
         */
        void clear()
        {
            cache_.clear();
        }

        /*!
         * \brief Limits the size of primary key cache size.
         *
         * \param n maximal size of cache
         */
        void resize(size_t n)
        {
            cache_.resize(n);
        }

        /*!
         * \brief Limits the depth of secondary key cache size.
         *
         * \param n maximal depth of cache
         */
        void setDepth(size_t n)
        {
            depth_ = n;
            for (auto& s : cache_)
                s.resize(n);
        }

        /*!
         * \brief Get the reserved capacity (depth) of secondary key cache size.
         *
         * \retuem maximal size of cache
         */
        size_t depth() const noexcept
        {
            return depth_ == 0 ? cache_.max_size() : depth_;
        }

        /*!
         * \brief Get the reserved capacity of primary key cache size.
         *
         * \retuem maximal size of cache
         */
        size_t capacity() const noexcept
        {
            return cache_.capacity();
        }

        /*!
         * \brief Get the total number of primary keys in cache.
         *
         * \return number of keys
         */
        size_t size() const noexcept { return cache_.size(); }

        /*!
         * \brief Check whatever the cache is empty.
         *
         * \return true if cache is empty, otherwise return false
         */
        bool empty() const noexcept { return cache_.empty(); }

        /*!
         * \brief Count the number of elements in cache for specified
         * primary and secondary key.
         *
         * \param key primary key
         * \param hint secondary key (hint)
         * \return number of items
         */
        size_t count(const _Key& key, const _Hint& hint) const noexcept
        {
            auto it = cache_.find(key);
            return (it != cache_.end() ? it->second.count(hint) : 0);
        }

        /*!
         * \brief Count the number of elements in cache for specified primary key.
         *
         * \param key primary key
         * \return number of items
         */
        size_t count(const _Key& key) const noexcept
        {
            auto it = cache_.find(key);
            return (it != cache_.end() ? it->second.size() : 0);
        }

        /*!
         * \brief Check if specified key is contained in cache_map.
         * \detail Same as count(key) > 0. Provided for convenience.
         * \param key key to search
         * \return true if key contained in cache, otherwise retirn false
         */
        bool contains(const _Key& key) const noexcept
        {
            return cache_.count(key) > 0;
        }

    private:
        lookup_type cache_;
        size_t depth_ = 0;
    };

} // end namespace Qt5Extra
