#pragma once
#include <algorithm>
#include <deque>

namespace Qt5Extra
{
    /*!
     * \brief The CacheLine<> class is implementation detail of cache_map class.
     * \detail The CacheLine class provide the flat-like map based on std::deque<>
     * class. The main purpose of this class is to maintain the second layer of
     * CacheMap cache, i.e. store the different states and values together and
     * gives the availablity to insert, remove, update them by specified key.
     * \tparam _Key type of secondary key (hint)
     * \tparam _Value type of stored value
     * \note this class is reenterant
     */
    template<class _Key, class _Value>
    class CacheLine
    {
    public:
        using key_type = _Key;
        using mapped_type = _Value;
        using value_type = std::pair<_Key, _Value>;
        using container_type = std::deque<value_type>;

        CacheLine() = default;
        CacheLine(const CacheLine&) = default;
        CacheLine(CacheLine&&) = default;
        CacheLine& operator=(const CacheLine&) = default;
        CacheLine& operator=(CacheLine&&) = default;

        explicit CacheLine(size_t n)
            : capacity_(n)
        {}

        template<class... _Args>
        void emplace(const _Key& key, _Args&&... args)
        {
            auto it = search(key);
            if (it != storage_.end())
            {
                *it = { key, mapped_type(std::forward<_Args>(args)...) };
                return;
            }

            storage_.emplace_front(key, _Value(std::forward<_Args>(args)...));

            // shrink
            if (capacity_ > 0 && storage_.size() > capacity_)
                storage_.pop_back();
        }

        size_t erase(const _Key& key)
        {
            auto it = search(key);
            if (it == storage_.end())
                return 0;

            std::swap(*it, storage_.back());
            it = --storage_.end();
            storage_.erase(it);

            return 1;
        }

        void resize(size_t n)
        {
            while (storage_.size() > n)
                storage_.pop_back();

            capacity_ = n;
        }

        void clear() { storage_.clear(); }

        bool find(const _Key& key, _Value& result) const
        {
            auto it = search(key);
            if (it == storage_.end())
                return false;

            result = it->second;
            return true;
        }

        template<class _Pred>
        bool find_if(_Pred pred, _Value& result) const
        {
            auto it = this->locate(storage_.begin(), storage_.end(), pred);
            if (it == storage_.end())
                return false;

            result = it->second;
            return true;
        }

        bool contains(const _Key& key) const noexcept
        {
            return count(key) > 0;
        }

        size_t count(const _Key& key) const noexcept
        {
            auto it = search(key);
            return static_cast<size_t>(it != storage_.end());
        }

        size_t capacity() const noexcept { return capacity_; }

        size_t size() const noexcept { return storage_.size(); }

        bool empty() const noexcept { return storage_.empty(); }

    private:
        auto search(const _Key& k) const noexcept
        {
            return locate(storage_.begin(), storage_.end(), [k](const auto& e) { return k == e; });
        }

        auto search(const _Key& k) noexcept
        {
            return locate(storage_.begin(), storage_.end(), [k](const auto& e) { return k == e; });
        }

        template<class _It, class _Pred>
        static auto locate(_It first, _It last, _Pred pred) noexcept
        {
            return std::find_if(first, last, [&pred](const auto& e) { return pred(e.first); });
        }

    private:
        container_type storage_;
        size_t capacity_;
    };
} // end namespace Qt5Extra
