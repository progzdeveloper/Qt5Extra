#pragma once
#include <QDialog>
#include <QtPluginsExtra>

class QTreeWidgetItem;

class QtPluginManagerDialogPrivate;
class QTPLUGINSEXTRA_EXPORT QtPluginManagerDialog :
        public QDialog
{
    Q_OBJECT
public:
    explicit QtPluginManagerDialog(QWidget *parent = Q_NULLPTR);
    ~QtPluginManagerDialog();

private Q_SLOTS:
    void showProperties(QTreeWidgetItem* item);

private:
    friend class QtPluginManagerDialogPrivate;
    QScopedPointer<class QtPluginManagerDialogPrivate> d;
};


