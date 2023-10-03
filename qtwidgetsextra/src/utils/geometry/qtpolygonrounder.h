#include "distances.h"
#include <algorithm>
#include <QPainterPath>
#include <QPolygon>
#include <QtWidgetsExtra>

template<class _Distance = ManhattanDistance>
class QtPolygonRounder : public _Distance
{
public:

    QPainterPath operator()(const QRect& rect, double radius) const {
        return operator ()(QRectF(rect), radius);
    }

    QPainterPath operator()(const QRectF& rect, double radius) const {
        return operator ()(rect, radius);
    }

    QPainterPath operator()(const QPolygonF& polygon, double radius) const
    {
        QPainterPath path;

        const int n = polygon.size();
        if (n < 3) // unable to rounding a polygon with less than 3 vertices
        {
            path.addPolygon(polygon);
            return path;
        }

        radius = std::max(0.0, radius); // radius can't be negative

        //
        // Next piece of code is an optimized
        // version of code given in comments below.
        // Optimized version eliminates modulo
        // operations and unnecessary computations,
        // so it looks more bizarre.
        //

        double r;
        QPointF pt1, pt2, curr, next;

        curr = polygon[0];
        next = polygon[1];

        // process first point
        r = distanceRadius(curr, next, radius);
        pt1 = lineStart(curr, next, r);
        path.moveTo(pt1);
        pt2 = lineEnd(curr, next, r);
        path.lineTo(pt2);

        int i = 1;
        for (; i < n - 1; ++i)
        {
            curr = polygon[i];
            next = polygon[i + 1];

            r = distanceRadius(curr, next, radius);
            pt1 = lineStart(curr, next, r);
            path.quadTo(curr, pt1);
            pt2 = lineEnd(curr, next, r);
            path.lineTo(pt2);
        }

        // process last point
        curr = polygon[i];
        next = polygon[0];

        r = distanceRadius(curr, next, radius);
        pt1 = lineStart(curr, next, r);
        path.quadTo(curr, pt1);
        pt2 = lineEnd(curr, next, r);
        path.lineTo(pt2);

        // close last corner
        curr = polygon[0];
        next = polygon[1];
        r = distanceRadius(curr, next, radius);
        pt1 = lineStart(curr, next, r);
        path.quadTo(curr, pt1);

        return path;

         /* simplified and non-optimized version of the above code
          *
          * for (int i = 0; i < n; ++i)
          * {
          *     curr = polygon[i];
          *     next = polygon[(i+1)%n];
          *     pt1 = lineStart(curr, next, distanceRadius(curr, next, radius));
          *     if (i == 0)
          *         path.moveTo(pt1);
          *     else
          *         path.lineTo(pt1);//path.quadTo(curr, pt1);
          *     pt2 = lineEnd(curr, next, distanceRadius(curr, next, radius));
          *     path.lineTo(pt2);
          * }
          *
          *  // close last corner
          * curr = polygon[0];
          * next = polygon[1];
          * pt1 = lineStart(curr, next, distanceRadius(curr, next, radius));
          * path.lineTo(pt1);//path.quadTo(curr, pt1);
          * return path;
          */
    }

    QPainterPath operator()(const QPolygonF& polygon, const QVector<double>& radiuses) const {
        return operator()(polygon, radiuses.data(), radiuses.size());
    }

    template<class _Alloc>
    QPainterPath operator()(const QPolygonF& polygon, const std::vector<double, _Alloc>& radiuses) const {
        return operator()(polygon, radiuses.data(), radiuses.size());
    }

    template<int _Prealloc>
    QPainterPath operator()(const QPolygonF& polygon, const QVarLengthArray<double, _Prealloc>& radiuses) const {
        return operator()(polygon, radiuses.data(), radiuses.size());
    }

    template<size_t _Size>
    QPainterPath operator()(const QRect& rect, const double (&radiuses)[_Size]) const {
        static_assert (_Size <= 4, "reactangle can't have more than 4 corners");
        return operator()(QRectF(rect), radiuses, _Size);
    }

    template<size_t _Size>
    QPainterPath operator()(const QRectF& rect, const double (&radiuses)[_Size]) const {
        static_assert (_Size <= 4, "reactangle can't have more than 4 corners");
        return operator()(rect, radiuses, _Size);
    }

private:
    QPainterPath operator()(const QPolygonF& polygon, const double* radiuses, size_t length) const
    {
        QPainterPath path;
        const size_t n = static_cast<size_t>(polygon.size() - polygon.isClosed());
        length = std::min(length, n);
        if (n < 3 || !hasRadiuses(radiuses, length))
        {
            // unable to round off a polygon with less than 3 vertices
            // or if all radiuses is zero or if no radiuses was provided
            path.addPolygon(polygon);
            return path;
        }

        double r, radius0 = std::max(radiuses[0], 0.0);
        QPointF pt1, pt2, curr, next;

        curr = polygon[0];
        next = polygon[1];

        //process first point
        r = distanceRadius(curr, next, radius0);
        pt1 = lineStart(curr, next, r);
        path.moveTo(pt1);
        r = distanceRadius(curr, next, std::max(radiuses[1], 0.0));
        pt2 = lineEnd(curr, next, r);
        path.lineTo(pt2);

        size_t i = 1;
        for (; i < (n - 1); ++i)
        {
            curr = polygon[i];
            next = polygon[i + 1];
            if (i >= length)
            {
                path.lineTo(curr);
                path.lineTo(next);
                continue;
            }

            r = distanceRadius(curr, next, std::max(radiuses[i], 0.0));
            pt1 = lineStart(curr, next, r);
            path.quadTo(curr, pt1);
            r = distanceRadius(curr, next, std::max(radiuses[i + 1], 0.0));
            pt2 = lineEnd(curr, next, r);
            path.lineTo(pt2);
        }

        //process last point
        curr = polygon[i];
        next = polygon[0];

        r = distanceRadius(curr, next, std::max(radiuses[i], 0.0));
        pt1 = lineStart(curr, next, r);
        path.quadTo(curr, pt1);
        r = distanceRadius(curr, next, radius0);
        pt2 = lineEnd(curr, next, r);
        path.lineTo(pt2);

        // close last corner
        curr = polygon[0];
        next = polygon[1];
        r = distanceRadius(curr, next, radius0);
        pt1 = lineStart(curr, next, r);
        path.quadTo(curr, pt1);

        return path;
    }

    static bool hasRadiuses(const double* radiuses, size_t length)
    {
        if (length == 0)
            return true;

        for (const double* r = radiuses, *end = radiuses + length; r != end; ++r)
            if (!qFuzzyIsNull(*r))
                return true;

        return false;
    }

private:
    inline double distance(const QPointF& curr, const QPointF& next) const
    {
        return static_cast<const _Distance&>(*this)(curr, next);
    }

    inline double distanceRadius(const QPointF& curr, const QPointF& next, double radius) const
    {
        return std::min(0.5, radius / distance(curr, next));
    }

    inline QPointF lineStart(const QPointF& curr, const QPointF& next, double r) const
    {
        QPointF pt;
        pt.setX((1.0 - r) * curr.x() + r * next.x());
        pt.setY((1.0 - r) * curr.y() + r * next.y());
        return pt;
    }

    inline QPointF lineEnd(const QPointF& curr, const QPointF& next, double r) const
    {
        QPointF pt;
        pt.setX(r * curr.x() + (1.0 - r) * next.x());
        pt.setY(r * curr.y() + (1.0 - r) * next.y());
        return pt;
    }
};


struct QTWIDGETSEXTRA_EXPORT QtStarPolygonizer
{
    inline QPolygonF operator()(int sideCount, double factor, const QRectF& rect) const
    {
        QPolygonF result;
        (*this)(result, sideCount, factor, rect);
        return result;
    }

    QPolygonF& operator()(QPolygonF& starGeometry, quint32 sideCount, double factor, const QRectF& rect) const;
};

struct QTWIDGETSEXTRA_EXPORT QtStarPolygonizerF
{
    inline QPolygonF operator()(int sideCount, double factor, const QRectF& rect) const
    {
        QPolygonF result;
        (*this)(result, sideCount, factor, rect);
        return result;
    }

    QPolygonF& operator()(QPolygonF& starGeometry, quint32 sideCount, double factor, const QRectF& rect) const;
};
