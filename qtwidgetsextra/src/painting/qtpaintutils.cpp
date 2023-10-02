#include "qtpaintutils.h"


inline void drawCircularTextImpl(QPainter& painter, const QString &text, const QPainterPath &path, qreal stretch, qreal start, qreal factor)
{
    QPen pen = painter.pen();

    QFontMetricsF metrics(painter.fontMetrics());
    qreal offset = metrics.height() * factor;

    qreal percentIncrease = qreal(1) / (qreal)(text.size() + 1) * stretch;

    for ( auto it = text.begin(); it != text.end(); ++it )
    {
        start += percentIncrease;
        if(start > 1.0)
        {
            //qDebug() << "start_in_percent is over 1.0:" << start_in_percent;
            start -= 1.0;
        }
        QPointF point = path.pointAtPercent(start);
        qreal angle = path.angleAtPercent(start);   // Clockwise is negative

        painter.save();
        // Move the virtual origin to the point on the curve
        painter.translate(point);
        // Rotate to match the angle of the curve
        // Clockwise is positive so we negate the angle from above
        painter.rotate(-angle);
        // Translate rotated painter down to half of text height
        painter.translate(0, offset);
        // Draw a line width above the origin to move the text above the line
        // and let Qt do the transformations
        painter.drawText(QPoint(0, -pen.width()), QString(*it));
        painter.restore();
    }
}


inline void drawCurvedTextImpl(QPainter& painter, const QString &text, const QPainterPath &path, qreal stretch, qreal factor)
{
    QPen pen = painter.pen();

    qreal percentIncrease = qreal(1) / (qreal)(text.size() + 1) * stretch;
    qreal percent = 0;

    QFontMetricsF metrics(painter.fontMetrics());
    qreal offset = metrics.height() * factor;

    for ( auto it = text.begin(); it != text.end(); ++it )
    {
        //qreal offset = metrics.boundingRect(text[i]).height() * 0.5;

        percent += percentIncrease;

        QPointF point = path.pointAtPercent(percent);
        qreal angle = path.angleAtPercent(percent);   // Clockwise is negative

        painter.save();
        // Move the virtual origin to the point on the curve
        painter.translate(point);
        // Rotate to match the angle of the curve
        // Clockwise is positive so we negate the angle from above
        painter.rotate(-angle);
        // Translate rotated painter down to half of text height
        painter.translate(0, offset);
        // Draw a line width above the origin to move the text above the line
        // and let Qt do the transformations
        painter.drawText(QPoint(0, -pen.width()), QString(*it));
        painter.restore();
    }
}


QtPainter::QtPainter() :
    QPainter()
{
}

QtPainter::QtPainter(QPaintDevice *device) :
    QPainter(device)
{
}

QtPainter::~QtPainter()
{
}

void QtPainter::drawCurvedText(const QString &text,
                               const QPainterPath &path,
                               qreal stretch,
                               qreal factor)
{
    drawCurvedTextImpl(*this, text, path,
                     qBound(qreal(0), stretch, qreal(1)),
                     qBound(qreal(0), factor, qreal(1)));
}

void QtPainter::drawCircularText(const QString &text,
                                 qreal x, qreal y,
                                 qreal w, qreal h,
                                 bool clockwise,
                                 qreal stretch,
                                 qreal start,
                                 qreal factor)
{
    QPainterPath path;
    path.addEllipse(x, y, w, h);
    if (!clockwise)
        path = path.toReversed();

    drawCircularTextImpl(*this, text, path,
                       qBound(qreal(0), stretch, qreal(1)),
                       qBound(qreal(0), start, qreal(1)),
                       qBound(qreal(0), factor, qreal(1)));
}

void QtPainter::drawCircularText(const QString &text,
                                 const QPointF &p,
                                 const QSizeF &size,
                                 bool clockwise,
                                 qreal stretch,
                                 qreal start,
                                 qreal factor)
{
    QPainterPath path;
    path.addEllipse(p.x(), p.y(), size.width(), size.height());
    if (!clockwise)
        path = path.toReversed();

    drawCircularTextImpl(*this, text, path,
                       qBound(qreal(0), stretch, qreal(1)),
                       qBound(qreal(0), start, qreal(1)),
                       qBound(qreal(0), factor, qreal(1)));
}

void QtPainter::drawCircularText(const QString &text,
                                 const QRectF &rect,
                                 bool clockwise,
                                 qreal stretch,
                                 qreal start,
                                 qreal factor)
{

    QPainterPath path;
    path.addEllipse(rect.x(), rect.y(), rect.width(), rect.height());
    if (!clockwise)
        path = path.toReversed();
    drawCircularTextImpl(*this, text, path,
                       qBound(qreal(0), stretch, qreal(1)),
                       qBound(qreal(0), start, qreal(1)),
                       qBound(qreal(0), factor, qreal(1)));
}


QtPainterSaver::QtPainterSaver( QPainter * p )
    : painter( p )
{
    if ( painter )
        painter->save();
}

QtPainterSaver::QtPainterSaver( QPainter & p )
    : painter( &p )
{
    p.save();
}

QtPainterSaver::~QtPainterSaver() {
    if ( painter )
        painter->restore();
}

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
