#pragma once
#include <QFactoryInterface>
#include <QtTextExtra>

class QtLanguageDetector;
class QtLanguageDetectorPlugin;
class QTTEXTEXTRA_EXPORT QtLangDetectorFactory :
        public QFactoryInterface
{
    Q_DISABLE_COPY(QtLangDetectorFactory)
    QtLangDetectorFactory();

public:
    ~QtLangDetectorFactory();

    void registerDetector(QtLanguageDetectorPlugin* plugin);
    QtLanguageDetector* createDetector(const QString& backendName) const;
    QStringList keys() const Q_DECL_OVERRIDE;

    static QtLangDetectorFactory& instance();

private:
    QScopedPointer<class QtLangDetectorFactoryPrivate> d;
};
