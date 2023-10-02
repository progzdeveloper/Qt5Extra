#pragma once
#include <QAbstractButton>
#include <QtWidgetsExtra>

class QTWIDGETSEXTRA_EXPORT QtSwitchButton : public QAbstractButton
{
public:
    QtSwitchButton(QWidget* parent);
    ~QtSwitchButton();

    // QWidget interface
protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;

    // QAbstractButton interface
protected:
    bool hitButton(const QPoint &pos) const Q_DECL_OVERRIDE;

private:
    QScopedPointer<class QtSwitchButtonPrivate> d;
};
