#include "qtcolorutils.h"
#include <QtCore/QHash>
#include <QtGui/QPixmapCache>
#include <QtGui/QPainter>

QString standardColorName(const QColor &color)
{
    static QHash<QColor, QString> colorNames;
    if (colorNames.isEmpty())
    {
        QColor c;
        const QStringList names = QColor::colorNames();
        for (const auto& name : names)
        {
            c.setNamedColor(name);
            colorNames[c] = name;
        }
    }
    auto it = colorNames.constFind(color);
    return (it == colorNames.cend() ? color.name() : *it);
}

QPixmap colorPixmap(const QColor& color, const QSize& size)
{
    const QString uniqueName = "_qt_cached_" + color.name() +
                                QString::number(size.width()) +
                                QString::number(size.height());

    QPixmap pixmap(size);
    if (QPixmapCache::find(uniqueName, &pixmap))
        return pixmap;

    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setPen(Qt::NoPen);
    painter.setBrush(color);
    painter.drawRoundRect(pixmap.rect().adjusted(1, 1, -1, -1), 4, 4);
    return pixmap;
}
