#include "qtbadgeeffect.h"
#include <cstring>
#include <cstdlib>

#include <QVariant>
#include <QStyle>
#include <QTextOption>
#include <QFont>
#include <QPainter>
#include <QPainterPath>
#include <QApplication>

#include <QtGeometryAlgorithms>

class QtStyleOptionChip
{
public:
    QTextOption textOptions;
    QFont font;
    QMargins margins;
    QRect rect;
    QSize buttonSize;
    QPalette palette;
    QStyle::State state; // reserved
    int spacing = 2;
    bool closeable = false;

    void paint(QPainter* painter, const QString& text) const;
    void paint(QPainter* painter, const QPixmap& pixmap) const;
    void paint(QPainter* painter, const QPixmap& pixmap, const QString& text) const;
};


void QtStyleOptionChip::paint(QPainter *painter, const QString &text) const
{
    const double r = minSide(rect) * 0.5;

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(Qt::NoPen);
    painter->setBrush(palette.highlight());
    painter->drawRoundedRect(rect, r, r);
    painter->setFont(font);

    QRect textRect;

    QMargins m = margins;
    if (closeable)
        m.setRight(buttonSize.width()+ margins.right() + spacing);
    textRect = rect.marginsRemoved(m);

    painter->setPen(palette.highlightedText().color());
    // draw text
    painter->drawText(textRect, text, textOptions);

    if (closeable) {
        painter->setBrush(Qt::NoBrush);
        // TODO: draw close button here
        painter->drawEllipse(rect.right() - margins.right() - buttonSize.width()/2, rect.y() + (rect.height() / 2 - buttonSize.height() / 2), buttonSize.width(), buttonSize.height());
    }
    painter->restore();
}

void QtStyleOptionChip::paint(QPainter *painter, const QPixmap &pixmap) const
{
    const double r = minSide(rect) * 0.5;

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing|QPainter::SmoothPixmapTransform);
    painter->setPen(Qt::NoPen);
    painter->setClipping(true);

    QRectF targetRect{ (double)margins.left(), (double)margins.top(), r, r };

    QPainterPath path;
    path.addEllipse(targetRect);

    painter->setClipping(true);
    painter->setClipPath(path);

    painter->setPen(Qt::NoPen);
    painter->fillRect(targetRect, palette.highlight());
    if (!pixmap.isNull())
        painter->drawPixmap(targetRect, pixmap, pixmap.rect());

    painter->setClipping(false);
    painter->restore();
}

void QtStyleOptionChip::paint(QPainter *painter, const QPixmap &pixmap, const QString &text) const
{
    paint(painter, pixmap);
    paint(painter, text);
}



class QtBageEffectPrivate
{
public:
    QPixmap pixmap;
    QMargins margins;
    QSize maximumSize;
    char text[5];
    int value;
    Qt::Alignment alignment;
    QtStyleOptionChip styleOption;

    QtBageEffectPrivate() :
        margins({}),
        maximumSize(34, 16),
        value(-1),
        alignment(Qt::AlignTop|Qt::AlignLeft)
    {
        std::memset(text, 0, sizeof(text));
        styleOption.palette = qApp->palette();
        styleOption.closeable = false;
        styleOption.textOptions.setAlignment(Qt::AlignCenter);
        styleOption.font.setFamily("Robato");
        styleOption.font.setPointSize(8);
    }
};


QtBadgeEffect::QtBadgeEffect(QObject *parent)
    : QGraphicsEffect(parent)
    , d(new QtBageEffectPrivate)
{
}

QtBadgeEffect::~QtBadgeEffect() = default;

void QtBadgeEffect::setMaximumSize(const QSize &size)
{

    if (d->maximumSize == size)
        return;

    d->maximumSize = size;
    update();
}

QSize QtBadgeEffect::maximumSize() const
{

    return d->maximumSize;
}

void QtBadgeEffect::setMargins(const QMargins &margins)
{
    if (d->margins == margins)
        return;

    d->margins = margins;
    update();
}

QMargins QtBadgeEffect::margins() const
{
    return d->margins;
}

void QtBadgeEffect::setFont(const QFont &font)
{

    if (d->styleOption.font == font)
        return;

    d->styleOption.font = font;
    update();
}

QFont QtBadgeEffect::font() const
{

    return d->styleOption.font;
}

void QtBadgeEffect::setCounter(int value)
{

    if (value == d->value)
        return;
    d->value = value;
    if (value < 0) {
        d->value = -1;
        std::memset(d->text, 0, sizeof(d->text));
    } else {
        if (d->value > 999)
            std::strncpy(d->text, "999+", sizeof("999+")-1);
        else
            std::snprintf(d->text, sizeof(d->text), "%i", d->value);
        d->pixmap = QPixmap();
    }
    update();
}

int QtBadgeEffect::counter() const
{
    return d->value;
}

QString QtBadgeEffect::text() const
{
    return d->text;
}

void QtBadgeEffect::setIcon(const QPixmap &icon)
{
    if (icon.isNull())
        return;

    d->pixmap = icon;
    d->value = -1;
    std::memset(d->text, 0, sizeof(d->text));
    update();
}

QPixmap QtBadgeEffect::icon() const
{
    return d->pixmap;
}

void QtBadgeEffect::setValue(const QVariant &value)
{
    switch (value.type())
    {
    case QVariant::Pixmap:
        setIcon(value.value<QPixmap>());
        break;
    case QVariant::Image:
        setIcon(QPixmap::fromImage(value.value<QImage>()));
        break;
    case QVariant::Icon:
        setIcon(value.value<QIcon>().pixmap(d->maximumSize));
        break;
    case QVariant::Int:
        setCounter(value.toInt());
        break;
    default:
        d->pixmap = QPixmap();
        d->value = -1;
        std::memset(d->text, 0, sizeof(d->text));
        update();
        break;
    }
}

QVariant QtBadgeEffect::value() const
{
    return d->pixmap.isNull() ? QVariant(d->value) : QVariant(d->pixmap);
}

void QtBadgeEffect::setAlignment(Qt::Alignment align)
{
    if (d->alignment == align)
        return;

    d->alignment = align;
    update();
}

Qt::Alignment QtBadgeEffect::alignment() const
{
    return d->alignment;
}

void QtBadgeEffect::draw(QPainter *painter)
{
    if ((d->value < 1 && d->pixmap.isNull()) || !isEnabled())
        return drawSource(painter);

    const QPixmap pixmap = sourcePixmap(Qt::DeviceCoordinates);
    painter->drawPixmap(0, 0, pixmap);

    painter->save();
    const QRectF rect = boundingRect().marginsRemoved(d->margins);

    const double w = std::min(rect.width(), (qreal)d->maximumSize.width());
    const double h = std::min(rect.height(), (qreal)d->maximumSize.height());
    const QRectF target = alignedRect({ 0, 0, w, h }, rect, d->alignment);

    d->styleOption.rect = target.toRect();
    if (d->value < 0)
        d->styleOption.paint(painter, d->pixmap);
    else
        d->styleOption.paint(painter, QString::fromLatin1(d->text));

    painter->restore();
}
