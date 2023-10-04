#pragma once
#include <QDockWidget>

class QPaintEvent;

class QtDockWidgetTitleBarPrivate;

class QtDockWidgetTitleBar : 
        public QWidget
{
    Q_OBJECT
public:
    explicit QtDockWidgetTitleBar(QWidget *parent = 0, Qt::WindowFlags f = 0);
    ~QtDockWidgetTitleBar();

public Q_SLOTS:
    void closeWidget();
    void autoHide();
    void undock();

private Q_SLOTS:
    void updateButtons(bool autoHide);
    void clicked(int button);
    void changeFeatures(QDockWidget::DockWidgetFeatures features);

protected:
    void paintEvent(QPaintEvent *);

private:
    friend class QtDockWidgetTitleBarPrivate;
    QScopedPointer<class QtDockWidgetTitleBarPrivate> d;
};

