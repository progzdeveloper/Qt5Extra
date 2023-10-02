#pragma once
#include <QTreeWidget>

#include <QtPluginsExtra>

class QTreeWidgetItem;

class QtPluginTreeViewPrivate;

class QTPLUGINSEXTRA_EXPORT QtPluginTreeView :
        public QTreeWidget
{
    Q_OBJECT
public:
    explicit QtPluginTreeView(QWidget *parent = Q_NULLPTR);
    ~QtPluginTreeView();

    QString key(const QModelIndex& index) const;

    inline QModelIndex indexFromItem ( QTreeWidgetItem * item, int column = 0 ) const
    {
        return QTreeWidget::indexFromItem(item, column);
    }

private:
    QScopedPointer<class QtPluginTreeViewPrivate> d;
};

