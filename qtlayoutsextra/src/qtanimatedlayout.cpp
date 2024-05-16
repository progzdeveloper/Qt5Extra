#include "qtanimatedlayout.h"
#include "qtlayoututils.h"

#include <QWidget>
#include <QApplication>
#include <QAbstractAnimation>
#include <QScopedValueRollback>


class QtAnimatedLayoutPrivate
{
public:
    LayoutItemAnimationOptions options_;
    bool animationEnabled_ = false;
    bool animationAllowed_ = false;
};


QtAnimatedLayout::QtAnimatedLayout(QWidget* widget)
    : QLayout(widget)
    , d(new QtAnimatedLayoutPrivate)
{
}

QtAnimatedLayout::~QtAnimatedLayout() = default;

void QtAnimatedLayout::setAnimated(bool on)
{
    d->animationEnabled_ = on;
    d->animationAllowed_ = on;
    if (on)
        qApp->installEventFilter(this);
    else
        qApp->removeEventFilter(this);
}

bool QtAnimatedLayout::isAnimated() const
{
    return d->animationEnabled_;
}

bool QtAnimatedLayout::isAnimationAllowed() const
{
    return d->animationAllowed_;
}

void QtAnimatedLayout::setMinimizationMargins(const QMargins& margins)
{
    d->options_.margins = margins;
}

QMargins QtAnimatedLayout::minimizationMargins() const
{
    return d->options_.margins;
}

void QtAnimatedLayout::setEasingCurve(const QEasingCurve& curve)
{
    d->options_.easingCurve = curve;
}

QEasingCurve QtAnimatedLayout::easingCurve() const
{
    return d->options_.easingCurve;
}

void QtAnimatedLayout::setAnimationDuration(std::chrono::milliseconds duration)
{
    d->options_.duration = duration;
}

std::chrono::milliseconds QtAnimatedLayout::animationDuration() const
{
    return d->options_.duration;
}

void QtAnimatedLayout::setAnimationDurationAsInt(int ms)
{
    using milliseconds = std::chrono::milliseconds;
    using rep_type = milliseconds::rep;
    d->options_.duration = milliseconds{ static_cast<rep_type>(std::max(0, ms)) };
}

int QtAnimatedLayout::animationDurationAsInt() const
{
    return static_cast<int>(d->options_.duration.count());
}

QAbstractAnimation* QtAnimatedLayout::createAnimation(QLayout* parent, QLayoutItem* item, const QRect& rect, const QRect& target) const
{
    return createItemAnimation(parent, item, rect, target, d->options_);
}

QAbstractAnimation* QtAnimatedLayout::animateItem(QLayout* parent, QLayoutItem* item, const QRect& rect, const QRect& target) const
{
    if (QAbstractAnimation* animation = createItemAnimation(parent, item, rect, target, d->options_))
    {
        animation->start(QAbstractAnimation::DeleteWhenStopped);
        return animation;
    }
    return nullptr;
}

const LayoutItemAnimationOptions& QtAnimatedLayout::animationOptions() const
{
    return d->options_;
}

bool QtAnimatedLayout::eventFilter(QObject* watched, QEvent* event)
{
    QWidget* widget = parentWidget();
    if (d->animationAllowed_ && d->animationEnabled_ && watched == widget &&
            (event->type() == QEvent::Resize || event->type() == QEvent::Show))
    {
        QScopedValueRollback guard(d->animationAllowed_, false);
        QApplication::sendEvent(widget, event);
        return true;
    }
    return QObject::eventFilter(watched, event);
}
