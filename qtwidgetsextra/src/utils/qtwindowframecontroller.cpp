#include "qtwindowframecontroller.h"
#include <QPointer>
#include <QRubberBand>
#include <QEvent>
#include <QHoverEvent>
#include <QMouseEvent>

class QtWindowFrameControllerPrivate
{
public:
    QtWindowFrameController* q = nullptr;
    QPoint dragPos;
    QPointer<QWidget> target;
    QPointer<QWidget> watched;
    mutable QScopedPointer<QRubberBand> rubberBandWidget;
    QtWindowFrameController::ResizeMode resizeMode = QtWindowFrameController::ContinuousResize;
    QtWindowFrameController::Options options = QtWindowFrameController::Resizing;
    Qt::Edges mousePress = Qt::Edges{};
    Qt::Edges mouseMove = Qt::Edges{};
    int borderWidth = 5;
    bool cursorChanged = false;
    bool leftButtonPressed = false;
    bool dragStarted = false;
    bool enabled = true;

    QtWindowFrameControllerPrivate(QtWindowFrameController* ctrl)
        : q(ctrl)
    {}

    void calculateCursorPosition(const QPoint& pos, const QRect& frameRect, Qt::Edges& edge)
    {
        const int xLeft = frameRect.left();
        const int xRight = frameRect.right();
        const int yTop = frameRect.top();
        const int yBottom = frameRect.bottom();

        const bool onLeft = pos.x() >= xLeft - borderWidth && pos.x() <= xLeft + borderWidth &&
                            pos.y() <= yBottom - borderWidth && pos.y() >= yTop + borderWidth;

        const bool onRight = pos.x() >= xRight - borderWidth && pos.x() <= xRight &&
                             pos.y() >= yTop + borderWidth && pos.y() <= yBottom - borderWidth;

        const bool onBottom = pos.x() >= xLeft + borderWidth && pos.x() <= xRight - borderWidth &&
                              pos.y() >= yBottom - borderWidth && pos.y() <= yBottom;

        const bool onTop = pos.x() >= xLeft + borderWidth && pos.x() <= xRight - borderWidth &&
                           pos.y() >= yTop && pos.y() <= yTop + borderWidth;

        const bool onBottomLeft = pos.x() <= xLeft + borderWidth && pos.x() >= xLeft &&
                                  pos.y() <= yBottom && pos.y() >= yBottom - borderWidth;

        const bool onBottomRight = pos.x() >= xRight - borderWidth && pos.x() <= xRight &&
                                   pos.y() >= yBottom - borderWidth && pos.y() <= yBottom;

        const bool onTopRight = pos.x() >= xRight - borderWidth && pos.x() <= xRight &&
                                pos.y() >= yTop && pos.y() <= yTop + borderWidth;

        const bool onTopLeft = pos.x() >= xLeft && pos.x() <= xLeft + borderWidth &&
                               pos.y() >= yTop && pos.y() <= yTop + borderWidth;

        if (onLeft)
            edge = Qt::LeftEdge;
        else if (onRight)
            edge = Qt::RightEdge;
        else if (onBottom)
            edge = Qt::BottomEdge;
        else if (onTop)
            edge = Qt::TopEdge;
        else if (onBottomLeft)
            edge = Qt::BottomEdge | Qt::LeftEdge;
        else if (onBottomRight)
            edge = Qt::BottomEdge | Qt::RightEdge;
        else if (onTopRight)
            edge = Qt::TopEdge | Qt::RightEdge;
        else if (onTopLeft)
            edge = Qt::TopEdge | Qt::LeftEdge;
        else
            edge = Qt::Edges{};
    }

    QRect evaluateGeometry(const QPoint& globalPos)
    {
        int left = target->geometry().left();
        int top = target->geometry().top();
        int right = target->geometry().right();
        int bottom = target->geometry().bottom();
        switch (mousePress)
        {
        case Qt::TopEdge:
            top = globalPos.y();
            break;
        case Qt::BottomEdge:
            bottom = globalPos.y();
            break;
        case Qt::LeftEdge:
            left = globalPos.x();
            break;
        case Qt::RightEdge:
            right = globalPos.x();
            break;
        case Qt::TopEdge | Qt::LeftEdge:
            top = globalPos.y();
            left = globalPos.x();
            break;
        case Qt::TopEdge | Qt::RightEdge:
            right = globalPos.x();
            top = globalPos.y();
            break;
        case Qt::BottomEdge | Qt::LeftEdge:
            bottom = globalPos.y();
            left = globalPos.x();
            break;
        case Qt::BottomEdge | Qt::RightEdge:
            bottom = globalPos.y();
            right = globalPos.x();
            break;
        }
        return QRect(QPoint(left, top), QPoint(right, bottom));
    }

    QRect adjustRect(const QRect& rect, QWidget* widget) const
    {
        int left = rect.left();
        int top = rect.top();
        const int right = rect.right();
        const int bottom = rect.bottom();

        if (rect.width() < widget->minimumWidth())
            left = widget->geometry().x();

        if (rect.height() < widget->minimumHeight())
            top = widget->geometry().y();

        return QRect(QPoint(left, top), QPoint(right, bottom));
    }

    QMargins borderMargins() const
    {
        return { borderWidth, borderWidth, borderWidth, borderWidth };
    }

    QRubberBand* rubberBand() const
    {
        if (!rubberBandWidget)
            rubberBandWidget.reset(q->createRubberBand());
        return rubberBandWidget.get();
    }
};


QtWindowFrameController::QtWindowFrameController(QObject* parent)
    : QObject(parent)
    , d(new QtWindowFrameControllerPrivate(this))
{
}

QtWindowFrameController::~QtWindowFrameController() = default;

void QtWindowFrameController::setWidget(QWidget* target, QWidget* watched)
{
    if (d->target == target && d->watched == watched)
        return;

    if (watched == nullptr)
        watched = target;

    if (d->watched)
        d->watched->removeEventFilter(this);

    d->target = nullptr;
    if (!target)
        return;

    d->watched = watched;
    if (d->watched)
        d->watched->installEventFilter(this);

    if (!(target->windowFlags() & Qt::FramelessWindowHint))
        d->options &= ~Resizing;

    d->target = target;
    d->target->setMouseTracking(true);
    d->target->setAttribute(Qt::WA_Hover);
    d->rubberBand()->setMinimumSize(d->target->minimumSize());
}

QWidget* QtWindowFrameController::widget() const
{
    return d->target;
}

void QtWindowFrameController::setEnabled(bool on)
{
    if (d->enabled == on)
        return;
    d->enabled = on;
    if (!d->enabled)
        d->rubberBand()->hide();
    Q_EMIT enabledChanged(d->enabled, QPrivateSignal{});
}

bool QtWindowFrameController::isEnabled() const
{
    return d->enabled;
}

void QtWindowFrameController::setOptions(QtWindowFrameController::Options options)
{
    if (d->options == options)
        return;

    d->options = options;
    if (d->target && (d->options & RubberBand))
        d->rubberBand()->setMinimumSize(d->target->minimumSize());

    if (d->target && !(d->target->windowFlags() & Qt::FramelessWindowHint))
        d->options &= ~Resizing;

    Q_EMIT optionsChanged(d->options, QPrivateSignal{});
}

QtWindowFrameController::Options QtWindowFrameController::options() const
{
    return d->options;
}

void QtWindowFrameController::setResizeMode(ResizeMode mode)
{
    d->resizeMode = mode;
}

QtWindowFrameController::ResizeMode QtWindowFrameController::resizeMode() const
{
    return d->resizeMode;
}

void QtWindowFrameController::setBorderWidth(int width)
{
    if (width == d->borderWidth)
        return;

    d->borderWidth = width;
    Q_EMIT borderWidthChanged(d->borderWidth, QPrivateSignal{});
}

int QtWindowFrameController::borderWidth() const
{
    return d->borderWidth;
}

bool QtWindowFrameController::eventFilter(QObject* watched, QEvent* event)
{
    if (!d->enabled || watched != d->watched)
        return QObject::eventFilter(watched, event);

    switch (event->type())
    {
    case QEvent::HoverMove:
        return mouseHover(static_cast<QHoverEvent*>(event));
    case QEvent::Leave:
        return mouseLeave(event);
    case QEvent::MouseButtonPress:
        return mousePress(static_cast<QMouseEvent*>(event));
    case QEvent::MouseMove:
        return mouseMove(static_cast<QMouseEvent*>(event));
    case QEvent::MouseButtonRelease:
        return mouseRelease(static_cast<QMouseEvent*>(event));
    case QEvent::Hide:
        d->leftButtonPressed = false;
        d->dragStarted = false;
        d->rubberBand()->hide();
        d->target->unsetCursor();
        break;
    default:
        break;
    }
    return QObject::eventFilter(watched, event);
}

bool QtWindowFrameController::mouseHover(QHoverEvent* event)
{
    if (d->target)
        updateCursorShape(d->target->mapToGlobal(event->pos()));
    return false;
}

bool QtWindowFrameController::mouseLeave(QEvent*)
{
    if (!d->target)
        return false;

    if (!d->leftButtonPressed)
        d->target->unsetCursor();
    return true;
}

bool QtWindowFrameController::mousePress(QMouseEvent* event)
{
    if (!d->target || (event->button() != Qt::LeftButton))
        return false;

    d->leftButtonPressed = true;
    d->calculateCursorPosition(event->globalPos(), d->target->frameGeometry(), d->mousePress);
    if (d->options & Resizing && d->resizeMode == OnDemandResize)
        d->rubberBand()->setGeometry(d->target->geometry().marginsRemoved(d->borderMargins()));

    if (d->mousePress != Qt::Edges{})
    {
        d->rubberBand()->setGeometry(d->target->geometry());
        if ((d->options & RubberBand) && !d->rubberBand()->isVisible())
            d->rubberBand()->show();
    }
    if ((d->options & Dragging) && d->target->rect().marginsRemoved(d->borderMargins()).contains(event->pos()))
    {
        d->dragStarted = true;
        d->dragPos = event->globalPos();
    }

    if (!(d->options & Resizing))
        return ((d->options & ForwardMouseEvents) ? false : d->dragStarted);

    return d->mousePress != Qt::Edges{};
}

bool QtWindowFrameController::mouseRelease(QMouseEvent*)
{
    if (!d->target)
        return false;

    if (d->leftButtonPressed && (d->options & Resizing) && d->resizeMode == OnDemandResize)
        d->target->setGeometry(d->rubberBand()->geometry());

    d->leftButtonPressed = false;
    d->dragStarted = false;
    d->rubberBand()->hide();
    return false;
}

bool QtWindowFrameController::mouseMove(QMouseEvent* event)
{
    if (!d->target)
        return false;

    updateCursorShape(event->globalPos());
    if (!d->leftButtonPressed)
        return false;

    if (d->dragStarted)
    {
        const auto globalPos = event->globalPos();
        d->target->move(d->target->frameGeometry().topLeft() + (globalPos - d->dragPos));
        d->dragPos = globalPos;
    }

    if (!(d->options & Resizing))
        return ((d->options & ForwardMouseEvents) ? false : d->dragStarted);

    if (d->mousePress == Qt::Edges{})
        return false;

    const QRect rc = d->evaluateGeometry(event->globalPos());
    if (d->rubberBand()->isVisible())
        d->rubberBand()->setGeometry(d->adjustRect(rc, d->rubberBand()).marginsRemoved(d->borderMargins()));

    if (d->resizeMode == ContinuousResize)
        d->target->setGeometry(d->adjustRect(rc, d->target));

    event->ignore();
    return true;
}

void QtWindowFrameController::updateCursorShape(const QPoint& pos)
{
    if (!d->target)
        return;

    if (d->target->isFullScreen() || d->target->isMaximized())
    {
        if (d->cursorChanged)
            d->target->unsetCursor();
        return;
    }

    if (d->leftButtonPressed || d->borderWidth == 0)
        return;

    d->calculateCursorPosition(pos, d->target->frameGeometry(), d->mouseMove);
    d->cursorChanged = true;
    if ((d->mouseMove == Qt::TopEdge) || (d->mouseMove == Qt::BottomEdge))
    {
        d->target->setCursor(Qt::SizeVerCursor);
    }
    else if ((d->mouseMove == Qt::LeftEdge) || (d->mouseMove == Qt::RightEdge))
    {
        d->target->setCursor(Qt::SizeHorCursor);
    }
    else if ((d->mouseMove == (Qt::TopEdge | Qt::LeftEdge)) || (d->mouseMove == (Qt::BottomEdge | Qt::RightEdge)))
    {
        d->target->setCursor(Qt::SizeFDiagCursor);
    }
    else if ((d->mouseMove == (Qt::TopEdge | Qt::RightEdge)) || (d->mouseMove == (Qt::BottomEdge | Qt::LeftEdge)))
    {
        d->target->setCursor(Qt::SizeBDiagCursor);
    }
    else if (d->cursorChanged)
    {
        d->target->unsetCursor();
        d->cursorChanged = false;
    }
}

QRubberBand* QtWindowFrameController::createRubberBand() const
{
    return new QRubberBand(QRubberBand::Rectangle);
}
