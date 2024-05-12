#pragma once
#include <QPoint>
#include <QPointF>
#include <cmath>

struct EuclidDistance
{
    // precise, but not fast (since we have a square root computation)
    inline double operator()(const QPointF& p1, const QPointF& p2) const
    {
        const double dx = p1.x() - p2.x();
        const double dy = p1.y() - p2.y();
        const double d = dx * dx + dy * dy;
        return std::sqrt(d);
    }
};

struct ManhattanDistance
{
    // fast, but less accurate than EuclidianDistance
    inline double operator()(const QPointF& p1, const QPointF& p2) const
    {
        return std::abs(p1.x() - p2.x()) + std::abs(p1.y() - p2.y());
    }
};

