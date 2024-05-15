#include "qtlanguagedetector.h"
#include "qtlanguagedetectorfactory.h"

static QString globalPreferredDetector;
static QScopedPointer<QtLanguageDetector> globalDetectorInstance;

QStringList QtLanguageDetector::identify(const QStringRef &text) const
{
    if (!globalDetectorInstance)
        globalDetectorInstance.reset(QtLanguageDetectorFactory::instance().createDetector(globalPreferredDetector));

    return globalDetectorInstance ? globalDetectorInstance->identify(text) : QStringList{};
}

QStringList QtLanguageDetector::keys()
{
    return QtLanguageDetectorFactory::instance().keys();
}

void QtLanguageDetector::setPreferred(const QString &key)
{
    if (!keys().contains(key))
        return;

    globalPreferredDetector = key;
    globalDetectorInstance.reset(QtLanguageDetectorFactory::instance().createDetector(globalPreferredDetector));
}

QString QtLanguageDetector::preferred()
{
    return globalPreferredDetector;
}
