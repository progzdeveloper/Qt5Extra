#pragma once
#include <QSize>
#include <QRect>
#include <QVector>
#include <QVarLengthArray>
#include <algorithm>
#include <QtGeometry>


enum class RectFitPolicy
{
    StretchSource,
    CropSource
};

struct AdjustOption
{
    Qt::Alignment alignment = Qt::AlignCenter;
    Qt::AspectRatioMode mode = Qt::IgnoreAspectRatio;
    RectFitPolicy policy = RectFitPolicy::CropSource;
};

QTGEOMETRY_EXPORT QRect adjustedRect(const QRect& source, const QRect& bounds, const AdjustOption& option = {}) Q_DECL_NOTHROW;
QTGEOMETRY_EXPORT QRectF adjustedRect(const QRectF& source, const QRectF& bounds, const AdjustOption& option = {}) Q_DECL_NOTHROW;

QTGEOMETRY_EXPORT QRect alignedRect(const QRect& source, const QRect& bounds, Qt::Alignment align) Q_DECL_NOTHROW;
QTGEOMETRY_EXPORT QRectF alignedRect(const QRectF& source, const QRectF& bounds, Qt::Alignment align) Q_DECL_NOTHROW;

QTGEOMETRY_EXPORT Qt::Alignment quadrant(const QPoint& center, const QPoint& pos) Q_DECL_NOTHROW;
QTGEOMETRY_EXPORT Qt::Alignment quadrant(const QPointF& center, const QPointF& pos) Q_DECL_NOTHROW;

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


