#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QMdiArea;

class QtOverviewWidget;
class QtMainWindowManager;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public Q_SLOTS:
    void createCodeWindow();
    void createFsWindow();
private:
    void createCentralWidget();
    void createActions();
    void createDockWidgets();

private:
    QMenu* fileMenu;
    QAction* codeAct;
    QAction* viewAct;
    QAction* exitAct;
    QMdiArea* area;
    QtOverviewWidget* overview;
    QtMainWindowManager* manager;
};

#endif // MAINWINDOW_H
