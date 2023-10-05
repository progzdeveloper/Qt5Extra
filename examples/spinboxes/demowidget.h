#ifndef DEMOWIDGET_H
#define DEMOWIDGET_H

#include <QWidget>

class QFormLayout;
class QVBoxLayout;

class QtBadgeEffect;

class DemoWidget : public QWidget
{
    Q_OBJECT

public:
    DemoWidget(QWidget *parent = 0);
    ~DemoWidget();

    void insertWidget(QWidget* w);
    void insertWidgets(QWidget *label, QWidget* content);

public Q_SLOTS:
    void clicked(bool on);
    void setBageValue(int value);

private:
    QVBoxLayout* layout;
    QtBadgeEffect* bageEffect;
};

#endif // DEMOWIDGET_H
