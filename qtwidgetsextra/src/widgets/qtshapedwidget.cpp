#include "qtshapedwidget.h"
#include <QEvent>
#include <QStylePainter>
#include <QStyleOption>

class QtShapedWidgetPrivate
{
public:
    QtShapedWidget* q = nullptr;
    QtShapedWidget::MaskingOptions options = QtShapedWidget::TrackChildren | QtShapedWidget::IgnoreFrame;

    QtShapedWidgetPrivate(QtShapedWidget* widget)
        : q(widget)
    {}

    QRegion frameMask() const
    {
        QRegion mask = q->rect();
        const int w = q->frameWidth();
        mask -= q->rect().adjusted(w, w, -w, -w);
        return mask;
    }

    QRegion regionMask() const
    {
        QRegion mask;
        if (!(options & QtShapedWidget::IgnoreFrame) && q->frameWidth() != 0)
            mask = frameMask();

        const QList<QWidget*> children = q->findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly);
        for (const auto w : children)
        {
            if (!w->isVisible())
                continue;

            mask += widgetRegion(w);
        }
        return mask;
    }

    QRegion appendRegion(QWidget* widget, const QRegion& region) const
    {
        QRegion r = region;
        return (r += widgetRegion(widget));
    }

    QRegion removeRegion(QWidget* widget, const QRegion& region) const
    {
        QRegion r = region;
        return (r -= widgetRegion(widget));
    }

    QRegion widgetRegion(QWidget* widget) const
    {
        QRegion r = widget->mask();
        if ((options & QtShapedWidget::IgnoreMasks) || r.isEmpty())
            return widget->geometry();

        r.translate(widget->x(), widget->y());
        return r;
    }
};

QtShapedWidget::QtShapedWidget(QWidget* parent, Qt::WindowFlags flags)
    : QFrame(parent, flags)
    , d(new QtShapedWidgetPrivate(this))
{
}

QtShapedWidget::~QtShapedWidget() = default;

void QtShapedWidget::setOptions(QtShapedWidget::MaskingOptions options)
{
    if (d->options == options)
        return;

    d->options = options;
    updateMask();
}

QtShapedWidget::MaskingOptions QtShapedWidget::options() const
{
    return d->options;
}

bool QtShapedWidget::eventFilter(QObject* watched, QEvent* event)
{
    if ((d->options & TrackChildren) && watched->isWidgetType() && watched->parent() == this)
    {
        QWidget* widget = static_cast<QWidget*>(watched);
        switch (event->type())
        {
        case QEvent::ParentChange:
            if (!isAncestorOf(widget)) // we are not an ancestor of widget anymore
                widget->removeEventFilter(this);
            break;
        case QEvent::Show:
            setMask(d->appendRegion(widget, mask()));
            break;
        case QEvent::Hide:
            setMask(d->removeRegion(widget, mask()));
            break;
        case QEvent::Move:
        case QEvent::Resize:
            watched->event(event);
            setMask(d->regionMask());
            return true;
        default:
            break;
        }
    }
    return QWidget::eventFilter(watched, event);
}

void QtShapedWidget::paintEvent(QPaintEvent*)
{
    if (frameShape() == QFrame::NoFrame)
        return;

    QStyleOptionFrame opt;
    initStyleOption(&opt);

    QStylePainter painter(this);
    painter.drawPrimitive(QStyle::PE_Frame, opt);
}

void QtShapedWidget::resizeEvent(QResizeEvent* event)
{
    QFrame::resizeEvent(event);
    updateMask();
}

void QtShapedWidget::moveEvent(QMoveEvent* event)
{
    QFrame::moveEvent(event);
    updateMask();
}

void QtShapedWidget::childEvent(QChildEvent* event)
{
    QObject* object = event->child();
    if ((d->options & TrackChildren) &&
         event->added() &&
         object->isWidgetType() &&
         object->parent() == this)
    {
        object->removeEventFilter(this);
        object->installEventFilter(this);
    }
}

void QtShapedWidget::updateMask()
{
    setMask(d->regionMask());
}
