#pragma once
#include <QObject>

class QObject;

struct QtPluginMetadata
{
    QObject *instance;
    QString key;
    QString libPath;
    QString iid;
    QString errorString;
    bool isLoaded;
    bool isStatic;
};

