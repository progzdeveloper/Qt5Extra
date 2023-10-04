#pragma once
#include <QObject>
#include <QtPropertyBrowserExtra>

//#define QT_META_TR(ClassName) static const char *ClassName##MetaInfoTr[] =

class QtPropertyProxyObjectPrivate;

class QTPROPERTYBROWSER_EXPORT QtPropertyProxyObject :
    public QObject
{
    Q_OBJECT

public:
    explicit QtPropertyProxyObject(QObject *parent = Q_NULLPTR);
    virtual ~QtPropertyProxyObject();

public Q_SLOTS:
    virtual void apply();
    virtual void restoreDefaults();
};



