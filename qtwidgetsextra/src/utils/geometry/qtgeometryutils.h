#pragma once
#include <QSize>
#include <QRect>
#include <QVector>
#include <QVarLengthArray>
#include <algorithm>
#include <QtWidgetsExtra>

Q_DECL_CONSTEXPR int maxSide(int w, int h) Q_DECL_NOTHROW { return std::max(w, h); }
Q_DECL_CONSTEXPR int maxSide(double w, double h) Q_DECL_NOTHROW { return std::max(w, h); }
Q_DECL_CONSTEXPR int maxSide(const QSize& s) Q_DECL_NOTHROW { return maxSide(s.width(), s.height()); }
Q_DECL_CONSTEXPR int maxSide(const QSizeF& s) Q_DECL_NOTHROW { return maxSide(s.width(), s.height()); }
Q_DECL_CONSTEXPR int maxSide(const QRect& r) Q_DECL_NOTHROW { return maxSide(r.size()); }
Q_DECL_CONSTEXPR int maxSide(const QRectF& r) Q_DECL_NOTHROW { return maxSide(r.size()); }


Q_DECL_CONSTEXPR int minSide(int w, int h) Q_DECL_NOTHROW { return std::min(w, h); }
Q_DECL_CONSTEXPR int minSide(double w, double h) Q_DECL_NOTHROW { return std::min(w, h); }
Q_DECL_CONSTEXPR int minSide(const QSize& s) Q_DECL_NOTHROW { return minSide(s.width(), s.height()); }
Q_DECL_CONSTEXPR int minSide(const QSizeF& s) Q_DECL_NOTHROW { return minSide(s.width(), s.height()); }
Q_DECL_CONSTEXPR int minSide(const QRect& r) Q_DECL_NOTHROW { return minSide(r.size()); }
Q_DECL_CONSTEXPR int minSide(const QRectF& r) Q_DECL_NOTHROW { return minSide(r.size()); }

Q_DECL_CONSTEXPR int horizontalMargins(const QMargins& m) Q_DECL_NOTHROW { return m.left() + m.right(); }
Q_DECL_CONSTEXPR double horizontalMargins(const QMarginsF& m) Q_DECL_NOTHROW { return m.left() + m.right(); }

Q_DECL_CONSTEXPR int verticalMargins(const QMargins& m) Q_DECL_NOTHROW { return m.top() + m.bottom(); }
Q_DECL_CONSTEXPR double verticalMargins(const QMarginsF& m) Q_DECL_NOTHROW { return m.top() + m.bottom(); }

Q_DECL_CONSTEXPR QSize marginsSize(const QMargins& m) Q_DECL_NOTHROW { return { horizontalMargins(m), verticalMargins(m) }; }
Q_DECL_CONSTEXPR QSizeF marginsSize(const QMarginsF& m) Q_DECL_NOTHROW { return { horizontalMargins(m), verticalMargins(m) }; }

Q_DECL_CONSTEXPR QSize clampedSize(const QSize& size, const QSize& minSize, const QSize& maxSize) Q_DECL_NOTHROW
{
    return minSize.expandedTo(size).boundedTo(maxSize);
}

Q_DECL_CONSTEXPR QSizeF clampedSize(const QSizeF& size, const QSizeF& minSize, const QSizeF& maxSize) Q_DECL_NOTHROW
{
    return minSize.expandedTo(size).boundedTo(maxSize);
}

QTWIDGETSEXTRA_EXPORT QRect adjustedRect(const QRect& source, const QRect& bounds, Qt::Alignment alignment) Q_DECL_NOTHROW;
QTWIDGETSEXTRA_EXPORT QRectF adjustedRect(const QRectF& source, const QRectF& bounds, Qt::Alignment alignment) Q_DECL_NOTHROW;

QTWIDGETSEXTRA_EXPORT Qt::Alignment quadrant(const QPoint& center, const QPoint& pos) Q_DECL_NOTHROW;
QTWIDGETSEXTRA_EXPORT Qt::Alignment quadrant(const QPointF& center, const QPointF& pos) Q_DECL_NOTHROW;


struct GridSplitter
{
    void operator()(const QRect& source, int rows, int cols, QVector<QRect>& result)
    {
        result.resize(rows * cols);
        splitRects(source, rows, cols, result.data(), result.size());
    }

    template<size_t _Prealloc>
    void operator()(const QRect& source, int rows, int cols, QVarLengthArray<QRect, _Prealloc>& result)
    {
        result.resize(rows * cols);
        splitRects(source, rows, cols, result.data(), result.size());
    }

    template<uint _Rows, uint _Cols>
    void operator()(const QRect& source, QRect (&result)[_Rows * _Cols]) Q_DECL_NOTHROW
    {
        static_assert (_Rows > 1 && _Cols > 1, "_Rows and _Cols must be grater than 1");
        splitRects(source, { _Rows, _Cols }, result, _Rows * _Cols);
    }
private:
    static void splitRects(const QRect& source, int rows, int cols, QRect* result, int n) Q_DECL_NOTHROW;
};




