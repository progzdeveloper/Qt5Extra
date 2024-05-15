#pragma once
#include <QFactoryInterface>
#include <QtTextExtra>

class QtLanguageDetector;
class QtLanguageDetectorPlugin;
class QTTEXTEXTRA_EXPORT QtLanguageDetectorFactory :
        public QFactoryInterface
{
    Q_DISABLE_COPY(QtLanguageDetectorFactory)
    QtLanguageDetectorFactory();

public:
    ~QtLanguageDetectorFactory();

    void registerDetector(QtLanguageDetectorPlugin* plugin);
    QtLanguageDetector* createDetector(const QString& backendName) const;
    QStringList keys() const Q_DECL_OVERRIDE;

    static QtLanguageDetectorFactory& instance();

private:
    QScopedPointer<class QtLangDetectorFactoryPrivate> d;
};
