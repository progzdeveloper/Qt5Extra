#pragma once
#include <QToolBar>

class QtDockWidget;

class QtDockSideBarPrivate;

class QtDockSideBar :
        public QToolBar
{
    Q_OBJECT
public:
    explicit QtDockSideBar(Qt::DockWidgetArea area, QWidget *parent = 0);
    ~QtDockSideBar();

    void insertDockWidget(QtDockWidget *dw);
    void removeDockWidget(QtDockWidget *dw);
    bool contains(QtDockWidget *w) const;

private slots:
    void moveTab(int from, int to);
    void show(int key);

private:
    friend class QtDockSideBarPrivate;
    QScopedPointer<class QtDockSideBarPrivate> d;
};

