#pragma once
#include <Qt>
#include <QVector>
#include <QVarLengthArray>
#include <QRect>
#include <QRectF>
#include <QDebug>

#include <cmath>
#include <QtGeometryAlgorithms>

#include <array>
#include <type_traits>
#include <iterator>

struct QtRectLayout
{
    template<class T>
    static constexpr inline bool IsRectType = std::is_same_v<T, QRect> || std::is_same_v<T, QRectF>;

    template<class T>
    static constexpr inline bool IsSizeType = std::is_same_v<T, QSize> || std::is_same_v<T, QSizeF>;
};

struct QtGridRectLayout : public QtRectLayout
{
    template<class _Size>
    static _Size boundingSize(int rows, int cols, const _Size& itemSize)
    {
        static_assert(IsSizeType<_Size>, "only QSize/QSizeF types are supported");
        return _Size{ cols * itemSize.width(), rows * itemSize.height(), };
    }

    template<class _Size>
    static void minGridSize(const _Size& frameSize, const _Size& itemSize, int itemCount, int& rows, int& cols)
    {
        using scalar_type = decltype(frameSize.width());

        int maxRows = 0, maxCols = 0;
        maxGridSize(frameSize, itemSize, maxRows, maxCols);

        rows = maxRows, cols = maxCols;
        int count = itemCount;
        int minDiff = itemCount;
        // adjust grid size by brute-forcing all
        // possible row/column count combinations
        for (int i = 1; i < maxRows; ++i)
        {
            for (int j = 1; j < maxCols; ++j)
            {
                const int k = i * j;
                const int d = (k - count);
                const _Size s = boundingSize(i, j, itemSize);
                const bool fitInside = s.width() < frameSize.width() && s.height() < frameSize.height();
                if (fitInside && k >= count && d <= minDiff)
                {
                    minDiff = d;
                    rows = i;
                    cols = j;
                    if (minDiff == 0)
                        return; // ideal case
                }
            }
        }
    }


    template<class _Size>
    static void maxGridSize(const _Size& frameSize, const _Size& itemSize, int& rows, int& cols)
    {
        static_assert(IsSizeType<_Size>, "only QSize/QSizeF types are supported");
        using scalar_type = decltype(itemSize.width());
        auto w = itemSize.width();
        auto h = itemSize.height();
        if (w <= scalar_type(0))
            w = frameSize.width();
        if (h <= scalar_type(0))
            w = frameSize.height();
        rows = static_cast<int>(std::floor(frameSize.height() / h));
        cols = static_cast<int>(std::floor(frameSize.width() / w));
    }



    template<class _Rect>
    void layoutRects(const _Rect& source, uint rows, uint cols, uint k, QVector<_Rect>& result)
    {
        result.reserve(rows * cols);
        layoutRects(source, rows, cols, k, std::back_inserter(result));
    }

    template<class _Rect, int _Prealloc>
    void layoutRects(const _Rect& source, uint rows, uint cols, uint k, QVarLengthArray<_Rect, _Prealloc>& result)
    {
        result.reserve(rows * cols);
        layoutRects(source, rows, cols, k, std::back_inserter(result));
    }

    template<class _Rect, uint _Rows, uint _Cols>
    void layoutRects(const _Rect& source, _Rect (&result)[_Rows * _Cols], uint k = _Rows * _Cols)
    {
        static_assert (_Rows > 1 && _Cols > 1, "_Rows and _Cols must be grater than 1");
        layoutRects(source, { _Rows, _Cols }, k, result);
    }

    template<class _Rect, class _OutIt>
    static void layoutRects(const _Rect& source, uint rows, uint cols, uint k, _OutIt result)
    {
        static_assert(IsRectType<_Rect>, "only QRect/QRectF types are supported");
        using scalar_type = decltype(source.width());

        if (!result)
            return;

        const uint n = std::min(rows * cols, k);
        const auto w = source.width() / cols;
        const auto h = source.height() / rows;
        _Rect cellRect{ 0, 0, static_cast<scalar_type>(w), static_cast<scalar_type>(h) };

        int i = 0;
        int x = source.x(), y = source.y();
        for (uint r = 0; r < rows; ++r)
        {
            for (uint c = 0; c < cols; ++c, ++i, ++result)
            {
                if (i == n)
                    return;

                cellRect.moveTo(x + c * w, y + r * h);
                *result = cellRect;
            }
        }
    }
};


struct QtBoxRectLayout : public QtRectLayout
{
    struct Options
    {
        int spacing = 0;
        Qt::Alignment align = Qt::AlignCenter;
        Qt::Orientation orientation = Qt::Horizontal;
    };

    // calculate zero-aligned bounding rect for sequence of rects according to options
    template<class _InIt>
    static auto boundingRect(const Options& options, _InIt first, _InIt last)
    {
        using RectType = typename std::iterator_traits<_InIt>::value_type;
        static_assert(IsRectType<RectType>, "only QRect/QRectF types are supported");

        RectType result{ 0, 0, 0, 0 };
        if (first == last)
            return result;

        using scalar_type = decltype (result.x());

        const int sp = std::max(0, options.spacing);
        scalar_type w = 0, h = 0;
        int n = 0;
        if (options.orientation == Qt::Vertical)
        {
            for (; first != last; ++first, ++n)
            {
                h += first->height();
                w = std::max(w, first->width());
            }
            h += sp * (n - 1);
        }
        else
        {
            for (; first != last; ++first, ++n)
            {
                w += first->width();
                h = std::max(h, first->height());
            }
            w += sp * (n - 1);
        }
        return RectType{ 0, 0, w, h };
    }

    template<class... _Args>
    static auto boundingRectOf(const Options& options, const _Args&... rects)
    {
        std::array ilist{ rects... };
        return boundingRect(options, ilist.begin(), ilist.end());
    }

    // calculate bounding size of n rects with specified size according to options
    template<class _Size>
    static _Size boundingSize(const Options& options, int n, const _Size& itemSize)
    {
        static_assert(IsSizeType<_Size>, "only QSize/QSizeF types are supported");
        if (n < 1 || !itemSize.isValid())
            return {};

        const int sp = std::max(0, options.spacing);
        if (options.orientation == Qt::Vertical)
            return { itemSize.width(), n * itemSize.height() + sp * (n - 1) };
        else
            return { n * itemSize.width() + sp * (n - 1), itemSize.height() };
    }

    // layout sequence of rects to fit inside specified frame according to options
    template<class _Rect, class _InIt, class _OutIt>
    static _Rect layoutRects(const _Rect& frame, const Options& options, _InIt first, _InIt last, _OutIt out)
    {
        using InputValue = typename std::iterator_traits<_InIt>::value_type;
        using OutputValue = typename std::iterator_traits<_OutIt>::value_type;
        static_assert(IsRectType<_Rect> && IsRectType<InputValue> && IsRectType<OutputValue>, "only QRect/QRectF types are supported");

        if (first == last)
            return {};

        if (!frame.isValid())
        {
            qWarning() << "[BoxRectLayout] unable to layout: frame rect is not valid";
            return {};
        }

        _Rect itemRect;

        const int sp = std::max(0, options.spacing);
        int n = 0;
        auto x = frame.x(), y = frame.y();
        auto w = frame.width(), h = frame.height();
        if (options.orientation == Qt::Vertical)
        {
            h = 0;
            for (; first != last; ++first, ++out, y += sp, ++n)
            {
                const auto ih = first->height(); // item height
                itemRect.setRect(x, y, w, ih);
                *out = alignedRect(*first, itemRect, options.align);

                h += ih;
                y += ih;
            }
            h += sp * (n - 1);
        }
        else
        {
            w = 0;
            for (; first != last; ++first, ++out, x += sp, ++n)
            {
                const auto iw = first->width(); // item width
                itemRect.setRect(x, y, iw, h);
                *out = alignedRect(*first, itemRect, options.align);

                w += iw;
                x += iw;
            }
            w += sp * (n - 1);
        }
        return { frame.x(), frame.y(), w, h };
    }

    template<class _Rect, class... _Args>
    static _Rect layoutItems(const _Rect& frame, const Options& options, _Args&&... rects)
    {
        std::array r{ rects... };
        auto first = r.begin();
        auto last = r.end();
        _Rect bounds = layoutRects(frame, options, first, last, first);
        ((rects = *first++), ...);
        return bounds;
    }

    // arrange n rects inside specified frame according to options
    template<class _Rect, class _OutIt>
    static _OutIt arrangeRects(const _Rect& frame, const Options& options, int n, _OutIt out)
    {
        using scalar_type = decltype (frame.x());
        using OutputValue = typename std::iterator_traits<_OutIt>::value_type;
        static_assert(IsRectType<_Rect> && IsRectType<OutputValue>, "only QRect/QRectF types are supported");

        if (n < 1)
            return out;

        if (!frame.isValid())
        {
            qWarning() << "[BoxRectLayout] unable to arrange rects: frame is not valid";
            return out;
        }

        const int sp_cnt = n - 1; // number of spacings
        const scalar_type sp = std::max(0, options.spacing);
        auto x = frame.x(), y = frame.y();
        if (options.orientation == Qt::Vertical)
        {
            scalar_type ih = frame.height() / scalar_type(n); // item height
            if (n > 1)
                ih -= (sp / sp_cnt - 1);
            for (int i = 0; i < n; ++i, ++out)
            {
                *out = _Rect{ x, y, frame.width(), ih };
                y += ih + sp;
            }
        }
        else
        {
            scalar_type iw = frame.width() / scalar_type(n); // item width
            if (n > 1)
                iw -= (sp / sp_cnt - 1);
            for (int i = 0; i < n; ++i, ++out)
            {
                *out = _Rect{ x, y, iw, frame.height() };
                x += iw + sp;
            }
        }
        return out;
    }
};
