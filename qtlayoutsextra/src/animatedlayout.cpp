#include "animatedlayout.h"
#include "layoututils.h"

#include <QWidget>
#include <QApplication>
#include <QAbstractAnimation>
#include <QScopedValueRollback>


class AnimatedLayoutPrivate
{
public:
    LayoutItemAnimationOptions options_;
    bool animationEnabled_ = false;
    bool animationAllowed_ = false;
};


AnimatedLayout::AnimatedLayout(QWidget* widget)
    : QLayout(widget)
    , d(new AnimatedLayoutPrivate)
{
}

AnimatedLayout::~AnimatedLayout() = default;

void AnimatedLayout::setAnimated(bool on)
{
    d->animationEnabled_ = on;
    d->animationAllowed_ = on;
    if (on)
        qApp->installEventFilter(this);
    else
        qApp->removeEventFilter(this);
}

bool AnimatedLayout::isAnimated() const
{
    return d->animationEnabled_;
}

bool AnimatedLayout::isAnimationAllowed() const
{
    return d->animationAllowed_;
}

void AnimatedLayout::setMinimizationMargins(const QMargins& margins)
{
    d->options_.margins = margins;
}

QMargins AnimatedLayout::minimizationMargins() const
{
    return d->options_.margins;
}

void AnimatedLayout::setEasingCurve(const QEasingCurve& curve)
{
    d->options_.easingCurve = curve;
}

QEasingCurve AnimatedLayout::easingCurve() const
{
    return d->options_.easingCurve;
}

void AnimatedLayout::setAnimationDuration(std::chrono::milliseconds duration)
{
    d->options_.duration = duration;
}

std::chrono::milliseconds AnimatedLayout::animationDuration() const
{
    return d->options_.duration;
}

void AnimatedLayout::setAnimationDurationAsInt(int ms)
{
    using milliseconds = std::chrono::milliseconds;
    using rep_type = milliseconds::rep;
    d->options_.duration = milliseconds{ static_cast<rep_type>(std::max(0, ms)) };
}

int AnimatedLayout::animationDurationAsInt() const
{
    return static_cast<int>(d->options_.duration.count());
}

QAbstractAnimation* AnimatedLayout::createAnimation(QLayout* parent, QLayoutItem* item, const QRect& rect, const QRect& target) const
{
    return createItemAnimation(parent, item, rect, target, d->options_);
}

QAbstractAnimation* AnimatedLayout::animateItem(QLayout* parent, QLayoutItem* item, const QRect& rect, const QRect& target) const
{
    if (QAbstractAnimation* animation = createItemAnimation(parent, item, rect, target, d->options_))
    {
        animation->start(QAbstractAnimation::DeleteWhenStopped);
        return animation;
    }
    return nullptr;
}

const LayoutItemAnimationOptions& AnimatedLayout::animationOptions() const
{
    return d->options_;
}

bool AnimatedLayout::eventFilter(QObject* watched, QEvent* event)
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
