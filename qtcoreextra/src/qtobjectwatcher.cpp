#include "qtobjectwatcher.h"
#include <QEvent>
#include <QDebug>

QtObjectWatcher::QtObjectWatcher()
{

}

bool QtObjectWatcher::attachObject(QObject* object)
{
    if (!object)
        return false;

    if (object->thread() != thread())
    {
        // we can't attach object living in another
        // thread since it can be deleted in meantime,
        // that in turn cause multithreaded issues
        qCritical() << "[QtObjectWatcher] thread of " << object << "is not the same as QtObjectWatcher thread: unable to attach object";
        return false;
    }

    //use emplace to avoid double search
    auto result = objectSet.emplace(object);
    if (!result.second)
        return false; // duplicate found

    return associate(object); // connect destroyed signal and install event filter
}

bool QtObjectWatcher::detachObject(QObject* object)
{
    if (object && objectSet.erase(object) > 0)
        return dissociate(object); // disconnect signal and remove event filter

    return false;
}

bool QtObjectWatcher::associate(QObject* object)
{
    if (!QObject::connect(object, &QObject::destroyed, this, &QtObjectWatcher::detachObject))
    {
        qCritical() << "[QtObjectWatcher] failed to connect to QObject::destroyed signal of" << object;
        return false;
    }
    // install event filter on watched object
    // to detect object move to another thread
    object->installEventFilter(this);
    return true;
}

bool QtObjectWatcher::dissociate(QObject* object)
{
    object->removeEventFilter(this);
    return QObject::disconnect(object, &QObject::destroyed, this, &QtObjectWatcher::detachObject);
}

bool QtObjectWatcher::contains(QObject* object) const
{
    return objectSet.count(object) > 0;
}

bool QtObjectWatcher::empty() const
{
    return objectSet.empty();
}

int QtObjectWatcher::size() const
{
    return static_cast<int>(objectSet.size());
}

bool QtObjectWatcher::eventFilter(QObject *watched, QEvent *event)
{
    // test if some of watched objects was moved to another thread
    if (event->type() == QEvent::ThreadChange && contains(watched))
    {
        qCritical() << "[ObjectWatcher] thread of" << watched << "was changed: the object will be detached";
        detachObject(watched);
    }
    return QObject::eventFilter(watched, event);
}

bool QtObjectWatcher::event(QEvent *event)
{
    // watcher was moved to another thread
    if (event->type() == QEvent::ThreadChange)
    {
        qCritical() << "[ObjectWatcher] current thread was changed any watched objects will be detached";
        for (QObject* object : objectSet)
            dissociate(object);
        objectSet.clear();
    }
    return QObject::event(event);
}
