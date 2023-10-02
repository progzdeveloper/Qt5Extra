#include "qtdocksidebar.h"
#include "qtdocktabbar.h"
#include "qtdockwidget.h"

#include <QTabBar>
#include <QDockWidget>
#include <QMouseEvent>
#include <QIcon>
#include <QList>


inline QString dockSideBarTitle(Qt::DockWidgetArea area)
{
    switch(area) {
    case Qt::LeftDockWidgetArea:
        return "_qt_LeftSideBar";
    case Qt::RightDockWidgetArea:
        return "_qt_RightSideBar";
    case Qt::TopDockWidgetArea:
        return "_qt_TopSideBar";
    case Qt::BottomDockWidgetArea:
        return "_qt_BottomSideBar";
    default:
        break;
    }
    return "_qt_SideBar";
}


class QtDockSideBarPrivate
{
public:
    QtDockSideBar *q_ptr;
    QList<QtDockWidget*> widgets;
    QtDockTabBar *tabBar;
    QtDockSideBarPrivate(QtDockSideBar* q);
    void initUi(Qt::DockWidgetArea area);
};


QtDockSideBarPrivate::QtDockSideBarPrivate(QtDockSideBar *q) :
    q_ptr(q)
{
}

void QtDockSideBarPrivate::initUi(Qt::DockWidgetArea area)
{
    tabBar = new QtDockTabBar(area, q_ptr);

    q_ptr->insertWidget(0, tabBar);
    q_ptr->setMovable(false);
    q_ptr->setFloatable(false);

    QObject::connect(tabBar, &QtDockTabBar::tabClicked, q_ptr, &QtDockSideBar::show);
    QObject::connect(tabBar, &QtDockTabBar::tabHover, q_ptr,   &QtDockSideBar::show);
    QObject::connect(tabBar, &QtDockTabBar::tabMoved, q_ptr,   &QtDockSideBar::moveTab);
}


QtDockSideBar::QtDockSideBar(Qt::DockWidgetArea area, QWidget *parent) :
    QToolBar(dockSideBarTitle(area), parent),
    d(new QtDockSideBarPrivate(this))
{
    d->initUi(area);
    setMovable(false);
    setFloatable(false);
    hide();
    setObjectName("_qt_sideBar");
}

QtDockSideBar::~QtDockSideBar() = default;

void QtDockSideBar::insertDockWidget(QtDockWidget *dw)
{
    if (contains(dw))
        return;

    if (dw->isAutoHide()) {
        d->tabBar->addTab(dw->windowIcon(), dw->windowTitle());
        QToolBar::show();
    }
    d->widgets << dw;
}

void QtDockSideBar::removeDockWidget(QtDockWidget *dw)
{
     
    int i = d->widgets.indexOf(dw);

    if (i != -1) {
        d->widgets.removeAt(i);
        d->tabBar->removeTab(i);
    }

    if (d->tabBar->count() == 0)
        hide();
}

bool QtDockSideBar::contains(QtDockWidget *w) const
{
     
    return (d->widgets.indexOf(w) != -1);
}

void QtDockSideBar::moveTab(int from, int to)
{
     
    d->widgets.move(from, to);
}

void QtDockSideBar::show(int key)
{
     
    QList<QtDockWidget*>::iterator it = d->widgets.begin();
    for (; it != d->widgets.end(); ++it) {
        if ((*it)->isAutoHide())
            (*it)->hide();
    }
    if (key >= 0 && key < d->widgets.size()) {
        d->widgets[key]->show();
        d->widgets[key]->setFocus();
        QWidget *content = d->widgets[key]->widget();
        if (content)
            content->setFocus();
    }
}

