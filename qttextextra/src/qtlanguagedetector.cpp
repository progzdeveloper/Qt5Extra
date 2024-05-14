#include "qtlanguagedetector.h"
#include "qtlangdetectorfactory.h"

static QString globalPreferredDetector;
static QScopedPointer<QtLanguageDetector> globalDetectorInstance;

QStringList QtLanguageDetector::identify(const QStringRef &text) const
{
    if (!globalDetectorInstance)
        globalDetectorInstance.reset(QtLangDetectorFactory::instance().createDetector(globalPreferredDetector));

    return globalDetectorInstance ? globalDetectorInstance->identify(text) : QStringList{};
}

QStringList QtLanguageDetector::keys()
{
    return QtLangDetectorFactory::instance().keys();
}

void QtLanguageDetector::setPreferred(const QString &key)
{
    if (!keys().contains(key))
        return;

    globalPreferredDetector = key;
    globalDetectorInstance.reset(QtLangDetectorFactory::instance().createDetector(globalPreferredDetector));
}

QString QtLanguageDetector::preferred()
{
    return globalPreferredDetector;
}
