#include "qtpolygonrounder.h"
#include <QtMath>

enum StarPolygonPrecision
{
    LowPrecision,
    HighPrecision
};

template<int _Precision>
QPolygonF& starPolygon(QPolygonF& starGeometry, quint32 sideCount, double factor, const QRectF& rect)
{
    starGeometry.resize(sideCount);
    if (sideCount < 3)
        starGeometry.resize(3);
    else
        starGeometry.resize(sideCount*2);

    factor = qBound(0.0, factor, 1.0); // bound factor to [0...1]
    double angle = M_PI / 2; // start at the top
    const double diff = 2 * M_PI / starGeometry.size(); // angle step for each side

    const double rw = rect.width() * 0.5;
    const double rh = rect.height() * 0.5;

    const QPointF c = rect.center();

    // calculate vertices for each inner and outer side
    for (int i = 0; i < starGeometry.size(); ++i, angle += diff)
    {
        //double angleCos = std::cos(angle);
        //double angleSin = std::sin(angle);

        double angleCos = 0.0;
        double angleSin = 0.0;
        if constexpr (_Precision == HighPrecision)
        {
            angleCos = std::cos(angle);
            angleSin = std::sin(angle);
        }
        else
        {
            angleCos = qFastCos(angle);
            angleSin = qFastSin(angle);
        }

        if ( (i & 1) ) // for every odd vertex
        {
            starGeometry[i].rx() = c.x() + rw * angleCos;
            starGeometry[i].ry() = c.y() + rh * angleSin;
        }
        else // for every not odd vertex
        {
            starGeometry[i].rx() = c.x() + (factor * rw) * angleCos;
            starGeometry[i].ry() = c.y() + (factor * rh) * angleSin;
        }
    }

    return starGeometry;
}


QPolygonF &QtStarPolygonizer::operator()(QPolygonF &starGeometry, quint32 sideCount, double factor, const QRectF &rect) const
{
    return starPolygon<StarPolygonPrecision::LowPrecision>(starGeometry, sideCount, factor, rect);
}

QPolygonF &QtStarPolygonizerF::operator()(QPolygonF &starGeometry, quint32 sideCount, double factor, const QRectF &rect) const
{
    return starPolygon<StarPolygonPrecision::HighPrecision>(starGeometry, sideCount, factor, rect);
}
