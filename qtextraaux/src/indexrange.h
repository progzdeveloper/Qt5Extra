#pragma once

struct IndexRange
{
    int offset = -1;
    int length = -1;

    IndexRange() = default;

    IndexRange(int pos, int len)
        : offset(pos)
        , length(len)
    {}

    inline bool contains(int pos) const noexcept
    {
        return ((pos >= offset) && (pos < (offset + length)));
    }

    inline bool contains(const IndexRange& other) const noexcept
    {
        return contains(other.offset) && contains(other.offset + other.length - 1);
    }

    inline bool intersects(const IndexRange& other) const noexcept
    {
        return contains(other.offset) || contains(other.offset + other.length - 1);
    }

    inline bool touch(const IndexRange& other) const noexcept
    {
        return offset == (other.length - 1) || ((length - 1) == other.offset);
    }

    inline IndexRange& operator+=(int d) noexcept
    {
        offset += d;
        return *this;
    }

    inline IndexRange& operator-=(int d) noexcept
    {
        offset -= d;
        return *this;
    }

    inline bool operator==(const IndexRange& other) const noexcept
    {
        return offset == other.offset && length == other.offset;
    }

    inline bool operator!=(const IndexRange& other) const noexcept
    {
        return !(*this == other);
    }
};
