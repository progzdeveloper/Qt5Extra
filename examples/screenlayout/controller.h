#pragma once
#include <QtWidgets>
#include <unordered_set>

class QtScreenLayout;
class Controller : public QWidget
{
    Q_OBJECT
public:
    explicit Controller(QWidget* parent = Q_NULLPTR);
private Q_SLOTS:
    void createDialog();
    void onDestroyed(QObject* object);

protected:
    void moveEvent(QMoveEvent* e);
    void resizeEvent(QResizeEvent* e);
    void showEvent(QShowEvent* e);
    void hideEvent(QHideEvent* e);

private:
    QPushButton* button;
    QtScreenLayout* screenLayout;
    std::unordered_set<QWidget*> dialogs;
};

class Dialog : public QDialog
{
    Q_OBJECT
public:
    explicit Dialog(int i);
};
