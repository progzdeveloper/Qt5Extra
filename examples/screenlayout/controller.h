#pragma once
#include <QtWidgets>

class QtScreenLayout;
class Controller : public QWidget
{
    Q_OBJECT
public:
    explicit Controller(QWidget* parent = Q_NULLPTR);
private Q_SLOTS:
    void createDialog();
private:
    QPushButton* button;
    QtScreenLayout* screenLayout;
};
