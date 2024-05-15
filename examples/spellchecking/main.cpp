#include "widget.h"
#include <QApplication>

#include <QtPluginManager>
#include <QtPluginInterface>

#include <QtSpellCheckBackendPlugin>
#include <QtSpellCheckBackendFactory>
#include <QtSpellCheckEngine>

#include <QtLanguageDetectorPlugin>
#include <QtLanguageDetectorFactory>
#include <QtLanguageDetector>

#include <QtResource>

class QtSpellCheckBackendInterface :
        public QtGenericInterface<QtSpellCheckBackendPlugin>
{
    typedef QtGenericInterface<QtSpellCheckBackendPlugin> InterfaceBase;
    // QtPluginInterface interface
public:
    QtSpellCheckBackendInterface() :
        InterfaceBase(QObject::tr("Spell Checking")) {
    }

    bool resolve(QObject *instance) const Q_DECL_OVERRIDE
    {
        QtSpellCheckBackendPlugin* plugin = qobject_cast<QtSpellCheckBackendPlugin*>(instance);
        if (plugin)
        {
            QtSpellCheckBackendFactory::instance().registerBackend(plugin);
            return true;
        }
        return false;
    }
};
QT_PLUGIN_INTERFACE(QtSpellCheckBackendInterface)


class QtLanguageDetectorInterface :
        public QtGenericInterface<QtLanguageDetectorPlugin>
{
    typedef QtGenericInterface<QtLanguageDetectorPlugin> InterfaceBase;
    // QtPluginInterface interface
public:
    QtLanguageDetectorInterface() :
        InterfaceBase(QObject::tr("Language Detecting")) {
    }

    bool resolve(QObject *instance) const Q_DECL_OVERRIDE
    {
        QtLanguageDetectorPlugin* plugin = qobject_cast<QtLanguageDetectorPlugin*>(instance);
        if (plugin)
        {
            QtLanguageDetectorFactory::instance().registerDetector(plugin);
            return true;
        }
        return false;
    }
};
QT_PLUGIN_INTERFACE(QtLanguageDetectorInterface)

void setupApplication()
{
    QtResource appRc{ ":/application.json" };
    const QJsonDocument document = appRc.toJsonDocument();
    QCoreApplication::setOrganizationName(document["orgname"].toString());
    QCoreApplication::setOrganizationDomain(document["orgdomain"].toString());
    QCoreApplication::setApplicationName(document["name"].toString());
    QCoreApplication::setApplicationVersion(document["version"].toString());
}


int main(int argc, char *argv[])
{
    setupApplication();
    QtPluginManager::instance().setAutoLoad(true);

    QApplication app(argc, argv);
    QSettings settings;
    QtLanguageDetector::setPreferred(settings.value("spellchecking/langdetect").toString());
    QtSpellCheckEngine::instance().setPrefferedBackend(settings.value("spellchecking/backend").toString());
    QtSpellCheckEngine::instance().start();

    Widget w;
    w.show();

    return app.exec();
}

