#include "qtgeometryutils.h"

namespace
{

template<class _Rect>
_Rect adjustedRectangle(const _Rect& source, const _Rect& bounds, Qt::Alignment alignment)
{
    static constexpr Qt::AlignmentFlag alignments[] =
    {
        Qt::AlignLeft, Qt::AlignRight,
        Qt::AlignTop, Qt::AlignBottom,
        Qt::AlignVCenter, Qt::AlignHCenter
    };

    if (source.width() > bounds.width() && source.height() > bounds.height())
        return bounds;

    _Rect result = source;

    // adjust size
    result.setSize(result.size().boundedTo(bounds.size()));

    Qt::Alignment align = alignment == 0 ? Qt::AlignTop | Qt::AlignLeft : alignment;

    // adjust position
    for (const auto i : alignments)
    {
        if (!(align & i))
            continue;

        switch(i)
        {
        case Qt::AlignTop:
            result.moveTop(bounds.top());
            break;
        case Qt::AlignBottom:
            result.moveBottom(bounds.bottom());
            break;
        case Qt::AlignLeft:
            result.moveLeft(bounds.left());
            break;
        case Qt::AlignRight:
            result.moveRight(bounds.right());
            break;
        case Qt::AlignVCenter:
            result.moveTop(bounds.top() + bounds.height() / 2 - result.height() / 2);
            break;
        case Qt::AlignHCenter:
            result.moveLeft(bounds.left() + bounds.width() / 2 - result.width() / 2);
            break;
        default:
            break;
        }
    }

    return result;
}

template<class _Point>
Qt::Alignment quadrantAt(const _Point& center, const _Point& pos) Q_DECL_NOTHROW
{
    Qt::Alignment align{0};
    if (pos == center)
        return align;

    if (pos.x() >= center.x())
        align |= Qt::AlignRight;
    if (pos.x() < center.x())
        align |= Qt::AlignLeft;
    if (pos.y() >= center.y())
        align |= Qt::AlignBottom;
    if (pos.y() < center.y())
        align |= Qt::AlignTop;

    return align;
}

}

QRect adjustedRect(const QRect& source, const QRect& bounds, Qt::Alignment alignment) Q_DECL_NOTHROW
{
    return adjustedRectangle(source, bounds, alignment);
}

QRectF adjustedRect(const QRectF& source, const QRectF& bounds, Qt::Alignment alignment) Q_DECL_NOTHROW
{
    return adjustedRectangle(source, bounds, alignment);
}

Qt::Alignment quadrant(const QPoint& center, const QPoint& pos) Q_DECL_NOTHROW
{
    return quadrantAt(center, pos);
}

Qt::Alignment quadrant(const QPointF& center, const QPointF& pos) Q_DECL_NOTHROW
{
    return quadrantAt(center, pos);
}

void GridSplitter::splitRects(const QRect& source, int rows, int cols, QRect* result, int n) Q_DECL_NOTHROW
{
    if (!result)
        return;

    const int w = source.width() / rows;
    const int h = source.height() / cols;
    QRect cellRect{0, 0, w, h };

    int i = 0;
    int x = source.x(), y = source.y();
    for (int r = 0; r < rows; ++r)
    {
        for (int c = 0; c < cols; ++c, ++i, ++result)
        {
            if (i == n)
                return;

            cellRect.moveTo(x + r * w, y + c * h);
            *result = cellRect;
        }
    }
}
