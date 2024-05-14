#pragma once
#include <QObject>
#include <QStringList>

class QtLanguageDetector
{
    Q_GADGET

public:
    virtual ~QtLanguageDetector() = default;

    virtual QStringList identify(const QStringRef &text) const;

    static QStringList keys();

    static void setPreferred(const QString& key);
    static QString preferred();
};

