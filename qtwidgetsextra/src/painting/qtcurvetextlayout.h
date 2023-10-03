#pragma once
#include <QPointF>
#include <QRectF>
#include <QtWidgetsExtra>

class QPainter;
class QPainterPath;

class QTWIDGETSEXTRA_EXPORT QtCurveTextLayout
{
public:
    explicit QtCurveTextLayout(QPainter& p);

    void drawCurvedText(const QString& text,
                        const QPainterPath& path,
                        qreal stretch = 0.5,
                        qreal factor = 0.5);

    void drawCircularText(const QString& text,
                          qreal x, qreal y,
                          qreal w, qreal h,
                          bool clockwise = true,
                          qreal stretch = 0.5,
                          qreal start = 0,
                          qreal factor = 0.5);

    void drawCircularText(const QString& text,
                          const QPointF &p,
                          const QSizeF &size,
                          bool clockwise = true,
                          qreal stretch = 0.5,
                          qreal start = 0,
                          qreal factor = 0.5);

    void drawCircularText(const QString& text,
                          const QRectF& rect,
                          bool clockwise = true,
                          qreal stretch = 0.5,
                          qreal start = 0,
                          qreal factor = 0.5);

private:
    QPainter& painter;
};
