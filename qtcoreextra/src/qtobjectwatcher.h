#pragma once
#include <QtCoreExtra>
#include <QObject>
#include <unordered_set>

class QTCOREEXTRA_EXPORT QtObjectWatcher
    : public QObject
{
    Q_OBJECT
public:
    template<class>
    friend class ObjectWatcher;
    using ObjectSet = std::unordered_set<QObject*>;
private:
    QtObjectWatcher();
    bool attachObject(QObject* object);
    bool detachObject(QObject* object);
    bool contains(QObject* object) const;
    bool empty() const;
    int size() const;
protected:
    virtual bool associate(QObject* object);
    virtual bool dissociate(QObject* object);
protected:
    bool eventFilter(QObject* watched, QEvent* event);
    bool event(QEvent* event);
private:
    ObjectSet objectSet;
};
