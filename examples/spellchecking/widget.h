#pragma once
#include <QtWidgets>

class Widget : public QWidget
{
    Q_OBJECT
public:
    explicit Widget(QWidget* parent = Q_NULLPTR);

private Q_SLOTS:
    void aboutPlugins();

private:
    QLineEdit* lineEdit;
    QTextEdit* textEdit;
    QToolButton* button;
};

