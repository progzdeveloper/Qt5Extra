#pragma once
#include <QSpinBox>
#include <QtWidgetsExtra>

class QTWIDGETSEXTRA_EXPORT QtSpinBoxEdit : public QSpinBox
{
    Q_OBJECT
public:
    explicit QtSpinBoxEdit(QWidget* parent = Q_NULLPTR);
    ~QtSpinBoxEdit();

public Q_SLOTS:
    virtual void showEditor();

private:
    QScopedPointer<class QtSpinBoxEditPrivate> d;
};
