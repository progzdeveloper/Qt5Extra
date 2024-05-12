#include "widget.h"
#include <QApplication>

#include <QtPluginManager>
#include <QtPluginInterface>
#include <QtSpellCheckBackendPlugin>
#include <QtSpellCheckBackendFactory>

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


int main(int argc, char *argv[])
{
    QtPluginManager::instance().setAutoLoad(true);

    QApplication app(argc, argv);

    Widget w;
    w.show();

    return app.exec();
}

