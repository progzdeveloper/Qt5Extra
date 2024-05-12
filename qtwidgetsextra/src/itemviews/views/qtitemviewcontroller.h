#pragma once
#include <QObject>

#include <QtWidgetsExtra>

class QAction;
class QAbstractItemView;

class QtActionItemDelegate;

class QTWIDGETSEXTRA_EXPORT QtItemViewController :
        public QObject
{
    Q_OBJECT

public:
    enum ActionRole
    {
        CommitRole     = 0x00000001,
        RevertRole     = 0x00000002,
        InsertRole     = 0x00000004,
        RemoveRole     = 0x00000008,
        MoveUpRole     = 0x00000010,
        MoveDownRole   = 0x00000020,
        SortAscRole    = 0x00000040,
        SortDescRole   = 0x00000080,

        SortRoles      = SortAscRole|SortDescRole,
        MoveRoles      = MoveUpRole|MoveDownRole,

        AllRoles       = 0x000000FF
    };
    Q_DECLARE_FLAGS(ActionRoles, ActionRole)
    Q_FLAG(ActionRoles)

    explicit QtItemViewController(QAbstractItemView* view, QObject* parent = Q_NULLPTR);
    QtItemViewController(QAbstractItemView* view, ActionRoles actions, QObject* parent = Q_NULLPTR);
    ~QtItemViewController();

    void setRoles(ActionRoles actions);
    ActionRoles roles() const;

    void addAction(QAction* action);
    void addActions(const QList<QAction*>& actions);

    ActionRole role(QAction* action) const;
    QAction* action(ActionRole witch) const;
    QList<QAction*> actions() const;

    QtActionItemDelegate* createDelegate(QObject *parent = 0) const;

    void clear();
    void enableActions();

Q_SIGNALS:
    void triggered(QAction*);

public Q_SLOTS:
    void insertItem();
    void removeItem();
    void moveItemUp();
    void moveItemDown();
    void editItem();
    void sortAsc();
    void sortDesc();
    void commit();
    void revert();

private:
    QScopedPointer<class QtItemViewControllerPrivate> d;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(QtItemViewController::ActionRoles)
