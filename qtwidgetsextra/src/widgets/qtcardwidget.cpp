#include "qtcardwidget.h"
#include "../effects/qtbadgeeffect.h"

#include <QApplication>
#include <QVariant>
#include <QPainter>
#include <QLabel>
#include <QtTextLabel>
#include <QBoxLayout>

class QtCardWidgetPrivate
{
public:
    QtCardWidget* q;
    QLabel* avatarLabel = nullptr;
    QtTextLabel* textLabel = nullptr;
    QtTextLabel* commentLabel = nullptr;
    QBoxLayout* layout = nullptr;
    QtBadgeEffect* badgeEffect = nullptr;
    QPixmap pixmap;
    int roundness = 0;
    Qt::ToolButtonStyle layoutMode = Qt::ToolButtonTextUnderIcon;

    QtCardWidgetPrivate(QtCardWidget* w)
        : q(w)
    {}

    void initUi()
    {
        constexpr QSize kDefaultAvatarSize{ 124, 124 };
        constexpr QMargins kDefaultAvatarMargins{ 12, 12, 12, 12 };

        avatarLabel = new QLabel(q);
        avatarLabel->setFixedSize(kDefaultAvatarSize);
        avatarLabel->setContentsMargins(kDefaultAvatarMargins);
        avatarLabel->setScaledContents(true);
        avatarLabel->setAlignment(Qt::AlignCenter);

        badgeEffect = new QtBadgeEffect(avatarLabel);
        badgeEffect->setAlignment(Qt::AlignTop | Qt::AlignRight);
        avatarLabel->setGraphicsEffect(badgeEffect);

        textLabel = new QtTextLabel(q);
        textLabel->setMinimumWidth(kDefaultAvatarSize.width());
        textLabel->setTextAlign(Qt::AlignCenter);
        textLabel->setAlignment(Qt::AlignCenter);
        textLabel->setMaxLineCount(1);

        commentLabel = new QtTextLabel(q);
        commentLabel->setMinimumWidth(kDefaultAvatarSize.width());
        commentLabel->setTextAlign(Qt::AlignTop | Qt::AlignHCenter);
        commentLabel->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
        commentLabel->setMaxLineCount(3);

        QVBoxLayout* textLayout = new QVBoxLayout;
        textLayout->addWidget(textLabel);
        textLayout->addWidget(commentLabel);

        layout = new QBoxLayout(layoutDirection(layoutMode), q);
        layout->addWidget(avatarLabel, 0, Qt::AlignCenter);
        layout->addLayout(textLayout, 1);
    }

    static QBoxLayout::Direction layoutDirection(Qt::ToolButtonStyle style)
    {
        return style == Qt::ToolButtonTextBesideIcon ?
                    (qApp->isLeftToRight() ? QBoxLayout::LeftToRight : QBoxLayout::RightToLeft) :
                    QBoxLayout::TopToBottom;
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

QtBadgeEffect& QtCardWidget::badge()
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
    if (d->layoutMode == style)
        return;

    d->layoutMode = style;
    switch(d->layoutMode)
    {
    case Qt::ToolButtonIconOnly:
        d->avatarLabel->setVisible(true);
        d->textLabel->setVisible(false);
        d->commentLabel->setVisible(false);
        break;
    case Qt::ToolButtonTextOnly:
        d->avatarLabel->setVisible(false);
        d->textLabel->setVisible(true);
        d->commentLabel->setVisible(true);
        break;
    default:
        d->avatarLabel->setVisible(true);
        d->textLabel->setVisible(true);
        d->commentLabel->setVisible(true);
        break;
    }

    const auto direction = d->layoutDirection(style);
    if (direction != d->layout->direction())
    {
        switch(direction)
        {
        case QBoxLayout::LeftToRight:
            d->textLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            d->textLabel->setTextAlign(Qt::AlignLeft | Qt::AlignVCenter);
            d->commentLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
            d->commentLabel->setTextAlign(Qt::AlignLeft | Qt::AlignTop);
            break;
        case QBoxLayout::RightToLeft:
            d->textLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
            d->textLabel->setTextAlign(Qt::AlignRight | Qt::AlignVCenter);
            d->commentLabel->setAlignment(Qt::AlignRight | Qt::AlignTop);
            d->commentLabel->setTextAlign(Qt::AlignRight | Qt::AlignTop);
            break;
        case QBoxLayout::TopToBottom:
        case QBoxLayout::BottomToTop:
        default:
            d->textLabel->setAlignment(Qt::AlignCenter);
            d->textLabel->setTextAlign(Qt::AlignCenter);
            d->commentLabel->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
            d->commentLabel->setTextAlign(Qt::AlignTop | Qt::AlignHCenter);
            break;
        }
        d->layout->setDirection(direction);
    }
    Q_EMIT cardStyleChanged(style);
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
    Q_EMIT avatarRoundnessChanged(factor);
}

int QtCardWidget::avatarRoundness() const
{
    return d->roundness;
}

QtTextLabel *QtCardWidget::textLabel() const
{
    return d->textLabel;
}

QtTextLabel *QtCardWidget::commentLabel() const
{
    return d->commentLabel;
}

QLabel *QtCardWidget::avatarLabel() const
{
    return d->avatarLabel;
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



