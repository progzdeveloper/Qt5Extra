#pragma once
#include <unordered_map>
#include <list>

namespace Qt5Extra
{
    /*!
     * \brief The LRUCache<> class provide the LRU Cache,
     * with STL compilant interface
     *
     * \tparam _Key type of key
     * \tparam _Value type of value
     * \tparam _Hasher type of key hasher (defaulted to std::hash<_Key>)
     * \tparam _KeyEq type of key equality comparator (defaulted to std::equal_to<_Key>)
     * \tparam _Alloc type of memory allocator (defaulted to std::allocator<std::pair<const _Key, _Value>>)
     *
     */
    template<
        class _Key,
        class _Value,
        class _Hasher = std::hash<_Key>,
        class _KeyEq = std::equal_to<_Key>,
        class _Alloc = std::allocator<std::pair<const _Key, _Value>>
    >
    class LRUCache
    {
    public:
        using key_type = _Key;
        using mapped_type = _Value;
        using value_type = std::pair<const key_type, mapped_type>;
        using key_equal = _KeyEq;
        using hasher = _Hasher;

        using node_list = std::list<value_type, _Alloc>;

        using iterator = typename node_list::iterator;
        using const_iterator = typename node_list::const_iterator;

        using reference = typename node_list::reference;
        using const_reference = typename node_list::const_reference;

        using pointer = typename node_list::pointer;
        using const_pointer = typename node_list::const_pointer;

    private:
        // indirect key hashing
        struct key_hash : _Hasher
        {
            inline size_t operator()(const _Key* key) const
            {
                return key ? _Hasher::operator()(*key) : 0;
            }
        };

        // indirect key comparing
        struct key_comp : _KeyEq
        {
            inline bool operator()(const _Key* lhs, const _Key* rhs) const
            {
                return ((lhs && rhs) ? _KeyEq::operator()(*lhs, *rhs) : (lhs == rhs));
            }
        };

        using node_map = std::unordered_map<const key_type*, iterator, key_hash, key_comp>;

    public:
        static constexpr size_t kMaxPrealloc = 256;
        static constexpr size_t kUnlimited = 0;

        /* The copy constructor is prohibited
         * since it's useless and we definetly
         * don't want to copy heavy objects
         * inside cache storage.
         */
        LRUCache(const LRUCache&) = delete;
        LRUCache& operator=(const LRUCache&) = delete;

        /* The move construction and assignment
         * is NOT prohibited since it cheap
         */
        LRUCache(LRUCache&&) = default;
        LRUCache& operator=(LRUCache&&) = default;

        /*!
         * \brief LRUCache constructor
         * \param n maximal LRU Cache capacity
         */
        explicit LRUCache(size_t n = kUnlimited)
        {
            resize(n);
        }

        /*!
         * \brief resize change LRU Cache capacity
         * \param n maximal LRU Cache capacity
         */
        void resize(size_t n)
        {
            limit = n;
            if (limit == kUnlimited)
                return;

            lookup.reserve(std::min(kMaxPrealloc, n));
            shrink();
        }

        void shrink()
        {
            if (limit == kUnlimited)
                return;

            while(list.size() > limit)
                evict();
        }

        /*!
         * \brief operator [] index operator overload
         * \param key
         * \return
         * \warning use it with extra care, since entity for
         * specified key will be default created if no associated
         * key found, and as a side-effect some other entities
         * may be removed as cache capacity overflows
         */
        mapped_type& operator[](const key_type& key)
        {
            auto it = lookup.find(key_pointer(key));
            if (it != lookup.end())
                return it->second->second;

            shrink(); // evict last used element on overflow

            auto nodeIt = list.emplace(list.begin(), key, mapped_type{});
            lookup[key_pointer(nodeIt->first)] = nodeIt;
            return nodeIt->second;
        }

        /*!
         * \brief emplace insert by creating a new entity in-place
         * \param args... arguments to forward to the constructor of the element
         * \return returns a pair consisting of an iterator to the inserted
         * element, or the already existing element if no insertion happened,
         * and a bool denoting whether the insertion took place (true if
         * insertion happened, false if did not)
         */
        template<class... _Args>
        std::pair<iterator, bool> emplace(_Args&&... args)
        {
            value_type val(std::forward<_Args>(args)...);
            auto it = lookup.find(key_pointer(val.first));
            if (it != lookup.end())
                return { it->second, false };

            shrink(); // evict last used element on overflow

            auto nodeIt = list.emplace(list.begin(), std::forward<_Args>(args)...);
            lookup[key_pointer(nodeIt->first)] = nodeIt;
            return { nodeIt, true };
        }

        /*!
         * \brief move_font move entity with specified key to front
         * if it's found in cache
         * \param key key of entity
         * \return number of entities moved (0 or 1)
         * \note no copy/move are performed, only internal pointers
         * reassignments take place
         */
        size_t move_font(const key_type& key)
        {
            auto it = lookup.find(key_pointer(key));
            if (it == lookup.end())
                return 0;

            move_font(it->second);
            return 1;
        }

        /*!
         * \brief move_font move entity pointing by iterator to front
         * \param it iterator pointing to interesting entity
         * \return result iterator pointing to same element
         * \note no copy/move are performed, only internal pointers
         * reassignments take place
         */
        iterator move_font(iterator it)
        {
            list.splice(list.begin(), list, it);
            return list.begin();
        }

        /*!
         * \brief erase erases entity (if one exists) with specified key
         * \param key key of entity to erase
         * \return number of entities erased (0 or 1)
         */
        size_t erase(const key_type& key)
        {
            auto it = lookup.find(key_pointer(key));
            if (it != lookup.end())
            {
                auto nodeIt = it->second;
                lookup.erase(it);
                list.erase(nodeIt);
                return 1;
            }
            return 0;
        }

        /*!
         * \brief erase erases entity (if one exists) pointed by specified iterator
         * \param it iterator pointing to erasing entity
         * \return iterator following the last removed element
         */
        iterator erase(iterator it)
        {
            lookup.erase(key_pointer(it->first));
            return list.erase(it);
        }

        /*!
         * \brief clear clear the cache
         */
        void clear() noexcept
        {
            lookup.clear();
            list.clear();
        }

        /*!
         * \brief max_size return maximum available capacity of LRU
         * cache as-if it's unlimited in size
         * \return maximum available capacity of LRU cache
         */
        size_t max_size() const noexcept { return list.max_size(); }

        /*!
         * \brief capacity return current capacity of LRU cache
         * \return current capacity of LRU cache
         */
        size_t capacity() const noexcept { return limit == kUnlimited ? max_size() : limit; }

        /*!
         * \brief size return current number of entities in cache
         * \return current number of entities in cache
         */
        size_t size() const noexcept { return list.size(); }

        /*!
         * \brief empty check if cache is empty
         * \return true if cache is empty, otherwise return false
         */
        bool empty() const noexcept { return list.empty(); }

        /*!
         * \brief count return  number of entities in cache with specified key
         * \param key key to find
         * \return number of entities that matches the key
         */
        size_t count(const key_type& key) const noexcept { return lookup.count(key_pointer(key)); }

        /*!
         * \brief contains check if entity with specified key contained in cache
         * \param key key to find
         * \return true if entity with key contained in cahce, otherwise return false
         */
        bool contains(const key_type& key) const noexcept { return count(key) > 0; }

        /*!
         * \brief find search the LRU cache for entity with specified key
         * \param key key to find
         * \return constant iterator pointing to found entity, if entity is
         * contained in cache, otherwise return iterator pointing to the end of cache
         */
        const_iterator find(const key_type& key) const
        {
            auto it = lookup.find(key_pointer(key));
            return it != lookup.end() ? const_iterator{ it->second } : list.end();
        }

        /*!
         * \brief find search the LRU cache for entity with specified key
         * \param key key to find
         * \return mutable iterator pointing to found entity if entity is
         * contained in cache, otherwise return iterator pointing to the end of cache
         */
        iterator find(const key_type& key)
        {
            auto it = lookup.find(key_pointer(key));
            return it != lookup.end() ? it->second : list.end();
        }

        /*!
         * \brief front return mutable reference to recently inserted entity
         * \return reference to most recently inserted entity
         */
        reference front() { return list.front(); }

        /*!
         * \brief front return mutable reference to lastly inserted entity
         * \return reference to most lastly inserted entity
         */
        reference back() { return list.back(); }

        /*!
         * \brief front return constant reference to recently inserted entity
         * \return constant reference to most recently inserted entity
         */
        const_reference front() const { return list.front(); }

        /*!
         * \brief front return constant reference to lastly inserted entity
         * \return constant reference to most lastly inserted entity
         */
        const_reference back() const { return list.back(); }

        iterator begin() { return list.begin(); }
        iterator end() { return list.end(); }

        const_iterator begin() const { return list.begin(); }
        const_iterator end() const { return list.end(); }

        const_iterator cbegin() const { return list.begin(); }
        const_iterator cend() const { return list.end(); }

    private:
        /*!
         * \brief evict evict last used element from cache
         */
        void evict()
        {
            if (list.empty())
                return;

            lookup.erase(key_pointer(list.back().first));
            list.pop_back();
        }

        /*!
         * \brief key_pointer return non-const pointer to key
         * \param k reference to key
         * \return pointer to key
         */
        static const key_type* key_pointer(const key_type& k)
        {
            return std::addressof(k);
        }

    private:
        node_map lookup;
        node_list list;
        size_t limit = kUnlimited;
    };
} // end namespace Qt5Extra

