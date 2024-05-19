#include "qtcursoreventfilter.h"
#include <QCursor>
#include <QTimer>
#include <QAbstractScrollArea>
#include <QEvent>
#include <QPointer>
#include <QVector>

class QtCursorEventFilterPrivate
{
public:
    QVector<int> wakeupEvents;
    QCursor cachedCursor;
    QtCursorEventFilter* q;
    QPointer<QWidget> widget;
    QTimer* timer = nullptr;
    int delay = 5000; // 5s
    bool enabled = true;
    bool hidden = false;
    bool custom = false;

    QtCursorEventFilterPrivate(QtCursorEventFilter* filter)
        : q(filter)
        , wakeupEvents {
              // common widget events
              QEvent::Show,
              QEvent::Hide,
              QEvent::Close,
              QEvent::EnabledChange,
              QEvent::FocusOut,
              QEvent::FocusIn,
              // touch and tablet events
              QEvent::TouchBegin,
              QEvent::TabletPress,
              QEvent::TabletMove,
              // window state change events
              QEvent::WindowStateChange,
              QEvent::WindowBlocked,
              QEvent::WindowDeactivate,
              QEvent::ActivationChange,
              // keyboard/mouse events
              QEvent::KeyPress,
              QEvent::MouseButtonPress,
              QEvent::MouseButtonRelease,
              QEvent::MouseMove,
              QEvent::Wheel,
              QEvent::Leave,
              QEvent::Enter
        }
    {
        std::sort(wakeupEvents.begin(), wakeupEvents.end());
    }

    bool isWakeupEvent(int type) const
    {
        return std::binary_search(wakeupEvents.begin(), wakeupEvents.end(), type);
    }

    void createTimer()
    {
        timer = new QTimer(q);
        timer->setSingleShot(true);
        QObject::connect(timer, &QTimer::timeout, q, &QtCursorEventFilter::hideCursor);
    }

    void showCursor()
    {
        timer->stop();
        if (hidden)
        {
            hidden = false;
            if (widget)
            {
                if (widget->cursor().shape() != Qt::BlankCursor) // someone already has changed the cursor shape
                    return;

                if (custom)
                    widget->setCursor(cachedCursor);
                else
                    widget->unsetCursor();
            }
        }

        if (widget)
        {
            timer->setSingleShot(true);
            timer->start(delay);
        }
    }

    void hideCursor()
    {
        if (hidden)
            return;

        hidden = true;
        if (widget)
        {
            custom = widget->testAttribute(Qt::WA_SetCursor);
            if (custom)
                cachedCursor = widget->cursor();

            widget->setCursor(Qt::BlankCursor);
        }
    }

    void attachWidget(QWidget* w)
    {
        if (!w)
            return;

        w->installEventFilter(q);
        custom = w->testAttribute(Qt::WA_SetCursor);
        if (custom)
            cachedCursor = w->cursor();
    }

    void detachWidget(QWidget* w)
    {
        if (!w)
            return;

        w->removeEventFilter(q);
        w->unsetCursor();
        reset();
    }

    void reset()
    {
        custom = false;
        cachedCursor = {};
    }
};


QtCursorEventFilter::QtCursorEventFilter(QObject* parent)
    : QObject(parent)
    , d(new QtCursorEventFilterPrivate(this))
{
    d->createTimer();
}

QtCursorEventFilter::~QtCursorEventFilter() = default;

void QtCursorEventFilter::setWidget(QWidget* widget)
{
    if (d->widget == widget)
        return;

    d->detachWidget(d->widget);

    if (auto area = qobject_cast<QAbstractScrollArea*>(d->widget))
        d->widget = area->viewport();
    else
        d->widget = widget;

    d->attachWidget(d->widget);
}

QWidget *QtCursorEventFilter::widget() const
{
    return d->widget;
}

void QtCursorEventFilter::setWakeupEvents(const QVector<int>& events)
{
    d->wakeupEvents = events;
    std::sort(d->wakeupEvents.begin(), d->wakeupEvents.end());
}

QVector<int> QtCursorEventFilter::wakeupEvents() const
{
    return d->wakeupEvents;
}

void QtCursorEventFilter::setEnabled(bool on)
{
    d->enabled = on;
    if (!d->enabled)
    {
        d->showCursor();
        d->timer->stop();
    }
}

bool QtCursorEventFilter::isEnabled() const
{
    return d->enabled;
}

void QtCursorEventFilter::setDelay(int msec)
{
    d->delay = msec;
}

int QtCursorEventFilter::delay() const
{
    return d->delay;
}

void QtCursorEventFilter::showCursor()
{
    d->showCursor();
}

void QtCursorEventFilter::hideCursor()
{
    d->hideCursor();
}

bool QtCursorEventFilter::eventFilter(QObject* watched, QEvent* event)
{
    if (!d->enabled || d->widget != watched)
        return QObject::eventFilter(watched, event);

    if (event->type() == QEvent::CursorChange)
        Q_EMIT cursorChanged(d->widget->cursor().shape() != Qt::BlankCursor);
    else if (d->isWakeupEvent(event->type()))
        showCursor();

    return QObject::eventFilter(watched, event);
}
