#include "qtcardwidget.h"
#include <QVariant>
#include <QPainter>
#include <QPainterPath>
#include <QApplication>
#include <cstring>
#include <cstdlib>


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
    bool closeable;

    void paint(QPainter* painter, const QString& text) const;
    void paint(QPainter* painter, const QPixmap& pixmap) const;
    void paint(QPainter* painter, const QPixmap& pixmap, const QString& text) const;
};


void QtStyleOptionChip::paint(QPainter *painter, const QString &text) const
{
    int r = std::min(rect.width(), rect.height()) / 2;

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(Qt::NoPen);
    painter->setBrush(palette.highlight());
    painter->drawRoundedRect(rect, r, r);
    painter->setFont(font);

    QRect textRect;

    if (!closeable)
        textRect = rect.adjusted(margins.left(), margins.top(), -margins.right(), -margins.bottom());
    else
        textRect = rect.adjusted(margins.left(), margins.top(), -(buttonSize.width()+margins.right()), -margins.bottom());

    painter->setPen(palette.highlightedText().color());
    // draw text
    painter->drawText(textRect, text, textOptions);

    if (closeable) {
        painter->setBrush(Qt::NoBrush);
        // draw button
        painter->drawEllipse(rect.right() - margins.right() - buttonSize.width()/2, rect.y() + (rect.height() / 2 - buttonSize.height() / 2), buttonSize.width(), buttonSize.height());
    }
    painter->restore();
}

void QtStyleOptionChip::paint(QPainter *painter, const QPixmap &pixmap) const
{
    int r = std::min(rect.width()-margins.right(), rect.height()-margins.bottom());

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing|QPainter::SmoothPixmapTransform);
    painter->setPen(Qt::NoPen);
    painter->setClipping(true);
    QRectF targetRect(rect.x() + margins.left(), rect.y() + margins.top(), r, r);

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



class QtGraphicsBageEffectPrivate
{
public:
    QPixmap pixmap;
    QMargins margins;
    QSize maximumSize;
    char text[5];
    int value;
    Qt::Alignment alignment;
    QtStyleOptionChip styleOption;

    QtGraphicsBageEffectPrivate() :
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


QtGraphicsBadgeEffect::QtGraphicsBadgeEffect(QObject *parent)
    : QGraphicsEffect(parent)
    , d(new QtGraphicsBageEffectPrivate)
{
}

QtGraphicsBadgeEffect::~QtGraphicsBadgeEffect() = default;

void QtGraphicsBadgeEffect::setMaximumSize(const QSize &size)
{
     
    if (d->maximumSize == size)
        return;

    d->maximumSize = size;
    update();
}

QSize QtGraphicsBadgeEffect::maximumSize() const
{
     
    return d->maximumSize;
}

void QtGraphicsBadgeEffect::setMargins(const QMargins &margins)
{
    if (d->margins == margins)
        return;

    d->margins = margins;
    update();
}

QMargins QtGraphicsBadgeEffect::margins() const
{
    return d->margins;
}

void QtGraphicsBadgeEffect::setFont(const QFont &font)
{
     
    if (d->styleOption.font == font)
        return;

    d->styleOption.font = font;
    update();
}

QFont QtGraphicsBadgeEffect::font() const
{
     
    return d->styleOption.font;
}

void QtGraphicsBadgeEffect::setCounter(int value)
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

int QtGraphicsBadgeEffect::counter() const
{
     
    return d->value;
}

QString QtGraphicsBadgeEffect::text() const
{
     
    return d->text;
}

void QtGraphicsBadgeEffect::setIcon(const QPixmap &icon)
{
     
    if (icon.isNull())
        return;

    d->pixmap = icon;
    d->value = -1;
    std::memset(d->text, 0, sizeof(d->text));
    update();
}

QPixmap QtGraphicsBadgeEffect::icon() const
{
     
    return d->pixmap;
}

void QtGraphicsBadgeEffect::setValue(const QVariant &value)
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

QVariant QtGraphicsBadgeEffect::value() const
{
    return d->pixmap.isNull() ? QVariant(d->value) : QVariant(d->pixmap);
}

void QtGraphicsBadgeEffect::setAlignment(Qt::Alignment align)
{
     
    if (d->alignment == align)
        return;
    d->alignment = align;
    update();
}

Qt::Alignment QtGraphicsBadgeEffect::alignment() const
{
     
    return d->alignment;
}

void QtGraphicsBadgeEffect::draw(QPainter *painter)
{
    static constexpr Qt::AlignmentFlag alignments[] =
    {
        Qt::AlignLeft, Qt::AlignRight,
        Qt::AlignTop, Qt::AlignBottom,
        Qt::AlignVCenter, Qt::AlignHCenter
    };

    if ((d->value < 1 && d->pixmap.isNull()) || !isEnabled())
        return drawSource(painter);

    const QPixmap pixmap = sourcePixmap(Qt::DeviceCoordinates);
    painter->drawPixmap(0, 0, pixmap);

    painter->save();
    QRectF rect = boundingRect().marginsRemoved(d->margins);

    QRectF target = rect;
    target.setWidth(std::min(rect.width(), (qreal)d->maximumSize.width()));
    target.setHeight(std::min(rect.height(), (qreal)d->maximumSize.height()));
    QPointF pos;
    for (auto a : alignments)
    {
        if (!(a & d->alignment))
            continue;

        switch (a)
        {
        case Qt::AlignLeft:
            pos.rx() = rect.left();
            break;
        case Qt::AlignRight:
            pos.rx() = rect.right() - target.width();
            break;
        case Qt::AlignHCenter:
            pos.rx() = (rect.center().x() - target.width() / 2);
            break;
        case Qt::AlignTop:
            pos.ry() = rect.top();
            break;
        case Qt::AlignBottom:
            pos.ry() = rect.bottom() - target.height();
            break;
        case Qt::AlignVCenter:
            pos.ry() = (rect.center().y() - target.height() / 2);
            break;
        default:
            break;
        }
    }

    target.moveTo(pos);
    d->styleOption.rect = target.toRect();
    if (d->value < 0)
        d->styleOption.paint(painter, d->pixmap);
    else
        d->styleOption.paint(painter, QString::fromLatin1(d->text));

    painter->restore();
}


#include <QLabel>
#include <QBoxLayout>

class QtCardWidgetPrivate
{
public:
    QtCardWidget* q;
    QLabel* avatarLabel = nullptr;
    QLabel* textLabel = nullptr;
    QLabel* commentLabel = nullptr;
    QBoxLayout* layout = nullptr;
    QtGraphicsBadgeEffect* badgeEffect = nullptr;
    QPixmap pixmap;
    int roundness = 0;
    Qt::ToolButtonStyle layoutMode = Qt::ToolButtonTextBesideIcon;
    Qt::TextElideMode textElideMode = Qt::ElideRight;
    Qt::TextElideMode commentElideMode = Qt::ElideMiddle;

    QtCardWidgetPrivate(QtCardWidget* w)
        : q(w)
    {}

    void initUi()
    {
        avatarLabel = new QLabel(q);
        avatarLabel->setContentsMargins(12, 12, 12, 12);
        avatarLabel->setScaledContents(true);
        avatarLabel->setAlignment(Qt::AlignCenter);
        avatarLabel->setFixedSize(128, 128);

        badgeEffect = new QtGraphicsBadgeEffect(avatarLabel);
        badgeEffect->setAlignment(Qt::AlignTop|Qt::AlignRight);
        avatarLabel->setGraphicsEffect(badgeEffect);

        textLabel = new QLabel(q);
        commentLabel = new QLabel(q);

        QVBoxLayout* textLayout = new QVBoxLayout;
        textLayout->addWidget(textLabel);
        textLayout->addWidget(commentLabel);

        layout = new QBoxLayout(layoutDirection(layoutMode), q);
        layout->addWidget(avatarLabel, 0, Qt::AlignCenter);
        layout->addLayout(textLayout);
    }

    static QBoxLayout::Direction layoutDirection(Qt::ToolButtonStyle style)
    {
        return style == Qt::ToolButtonTextBesideIcon ? QBoxLayout::LeftToRight : QBoxLayout::TopToBottom;
    }

    static QPixmap processPixmap(const QPixmap& pixmap, int roundness)
    {
        if (pixmap.isNull())
            return {};

        QPixmap result(pixmap.size());
        result.fill(Qt::transparent);
        QPainterPath path = framePath(result.rect(), roundness);

        QPainter p(&result);
        p.setClipPath(path);
        p.drawPixmap(result.rect(), pixmap, pixmap.rect());
        return result;
    }

    static QPainterPath framePath(const QRect& rect, int roundness)
    {
        const int w = std::min(rect.width(), rect.height());
        QRect r{ QPoint{}, QSize{ w, w} };
        QPainterPath path; 
        if(roundness > 0)
        {
            path.addRoundedRect(rect, roundness, roundness);
        }
        else if (roundness < 0)
        {
            r.moveCenter(rect.center());
            path.addEllipse(r);
        }
        else // roundness == 0
        {
            path.addRect(rect);
        }
        return path;
    }
};


QtCardWidget::QtCardWidget(QWidget *parent)
    : QFrame(parent)
    , d(new QtCardWidgetPrivate(this))
{
    d->initUi();
}

QtCardWidget::~QtCardWidget() = default;

QtGraphicsBadgeEffect& QtCardWidget::badge()
{
    return *d->badgeEffect;
}

void QtCardWidget::setBadgeValue(const QVariant &value)
{
    d->badgeEffect->setValue(value);
}

QVariant QtCardWidget::badgeValue() const
{
    return d->badgeEffect->value();
}

void QtCardWidget::setCardStyle(Qt::ToolButtonStyle style)
{
    if (d->layoutDirection(style) != d->layout->direction())
        d->layout->setDirection(d->layoutDirection(style));
    d->layoutMode = style;
}

Qt::ToolButtonStyle QtCardWidget::cardStyle() const
{
    return d->layoutMode;
}

void QtCardWidget::setAvatarRoundness(int factor)
{
    if (d->roundness == factor)
        return;

    d->roundness = factor;
    d->avatarLabel->setPixmap(d->processPixmap(d->pixmap, factor));
}

int QtCardWidget::avatarRoundness() const
{
    return d->roundness;
}

void QtCardWidget::setTextAlignment(Qt::Alignment align)
{
    d->textLabel->setAlignment(align);
}

Qt::Alignment QtCardWidget::textAlignment() const
{
    return d->textLabel->alignment();
}

void QtCardWidget::setCommentAlignment(Qt::Alignment align)
{
    d->commentLabel->setAlignment(align);
}

Qt::Alignment QtCardWidget::commentAlignment() const
{
    return d->commentLabel->alignment();
}

void QtCardWidget::setCommentWordWrap(bool on)
{
    d->commentLabel->setWordWrap(on);
}

bool QtCardWidget::isCommentWordWrap() const
{
    return d->commentLabel->wordWrap();
}

void QtCardWidget::setAvatar(const QPixmap &pixmap)
{
    d->pixmap = pixmap;
    d->avatarLabel->setPixmap(d->processPixmap(pixmap, d->roundness));
}

void QtCardWidget::setAvatar(const QImage &image)
{
    setAvatar(QPixmap::fromImage(image));
}

QPixmap QtCardWidget::avatar() const
{
    return d->pixmap;
}

void QtCardWidget::setText(const QString &text)
{
    d->textLabel->setText(text);
}

QString QtCardWidget::text() const
{
    return d->textLabel->text();
}

void QtCardWidget::setComment(const QString &text)
{
    d->commentLabel->setText(text);
}

QString QtCardWidget::comment() const
{
    return d->commentLabel->text();
}



