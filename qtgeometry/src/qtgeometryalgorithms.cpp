#include "qtgeometryalgorithms.h"

namespace
{
    template<class _Rect>
    _Rect alignedRectangle(const _Rect& source, const _Rect& bounds, Qt::Alignment alignment) Q_DECL_NOTHROW
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

    template<class _Rect>
    _Rect adjustedRectangle(const _Rect& source, const _Rect& bounds, const AdjustOption& option) Q_DECL_NOTHROW
    {
        _Rect result = source;

        const bool widthOut = source.width() > bounds.width(); // width out of bounds
        const bool heightOut = source.height() > bounds.height(); // height out of bounds
        const bool isStretch = option.policy == RectFitPolicy::StretchSource;

        // adjust size according to mode and policy
        switch(option.mode)
        {
        case Qt::IgnoreAspectRatio:
            if (isStretch || (widthOut && heightOut))
                return bounds;
            result.setSize(result.size().boundedTo(bounds.size()));
            break;
        case Qt::KeepAspectRatio:
            if (isStretch || (widthOut || heightOut))
                result.setSize(result.size().scaled(bounds.size(), Qt::KeepAspectRatio));
            break;
        case Qt::KeepAspectRatioByExpanding:
            if (isStretch || (widthOut && heightOut))
                result.setSize(result.size().scaled(bounds.size(), Qt::KeepAspectRatioByExpanding));
            break;
        default:
            break;
        }

        const Qt::Alignment align = option.alignment == 0 ? Qt::AlignCenter : option.alignment;

        // align result rect position
        return alignedRectangle(result, bounds, align);
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


QRect alignedRect(const QRect& source, const QRect& bounds, Qt::Alignment alignment) Q_DECL_NOTHROW
{
    return alignedRectangle(source, bounds, alignment);
}

QRectF alignedRect(const QRectF& source, const QRectF& bounds, Qt::Alignment alignment) Q_DECL_NOTHROW
{
    return alignedRectangle(source, bounds, alignment);
}

Qt::Alignment quadrant(const QPoint& center, const QPoint& pos) Q_DECL_NOTHROW
{
    return quadrantAt(center, pos);
}

Qt::Alignment quadrant(const QPointF& center, const QPointF& pos) Q_DECL_NOTHROW
{
    return quadrantAt(center, pos);
}

