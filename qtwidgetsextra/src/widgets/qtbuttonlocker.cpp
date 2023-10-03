#include "qtbuttonlocker.h"
#include <QEvent>
#include <QMouseEvent>

#include <QAbstractButton>


class QtButtonLockerPrivate
{
public:
    QIcon lockIcon;
    QHash<QObject*, QIcon> buttonIcons;
};


QtButtonLocker::QtButtonLocker(QObject *parent)
    : QObject(parent)
    , d(new QtButtonLockerPrivate)
{
}

QtButtonLocker::~QtButtonLocker() = default;

void QtButtonLocker::setIcon(const QIcon &icon)
{
    d->lockIcon = icon;
}

QIcon QtButtonLocker::icon() const
{
    return d->lockIcon;
}

bool QtButtonLocker::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() != QEvent::MouseButtonDblClick &&
        event->type() != QEvent::MouseButtonPress)
    {
        // standard event processing
        return QObject::eventFilter(watched, event);
    }

    QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
    if (mouseEvent->buttons() & Qt::LeftButton)
    {
        QAbstractButton* button = qobject_cast<QAbstractButton*>(watched);
        if (button == Q_NULLPTR)
            return QObject::eventFilter(watched, event); // standard event processing

        if (button->isChecked() && event->type() == QEvent::MouseButtonPress)
            return true; // filter out the event

        if (event->type() == QEvent::MouseButtonDblClick)
        {
            if (button->isChecked())  // unlock the button
            {
                button->setChecked(false);
                button->setCheckable(false);
                unlock(button);
            }
            else // lock the button
            {
                if (!button->isCheckable())
                    button->setCheckable(true);
                button->setChecked(true);
                lock(button);
            }
            return true; // filter out the event
        }
    }
    // standard event processing
    return QObject::eventFilter(watched, event);
}

void QtButtonLocker::lock(QAbstractButton *button)
{
    if (d->lockIcon.isNull())
        return;

    QIcon icon = button->icon();
    if (!icon.isNull()) // save original icon
        d->buttonIcons[button] = icon;

    button->setIcon(d->lockIcon);
    // remove from buttonIcons upon target button destruction to avoid wasting memory
    connect(button, &QObject::destroyed, this, [this](QObject* btn){ d->buttonIcons.remove(btn); });
}

void QtButtonLocker::unlock(QAbstractButton *button)
{
    if (d->lockIcon.isNull())
        return;

    auto it = d->buttonIcons.find(button);
    if (it != d->buttonIcons.end())
    {
        button->setIcon(*it); // restore original icon
        d->buttonIcons.erase(it); // erase from mapping
    }
    else
    {
        button->setIcon({});
    }
}
