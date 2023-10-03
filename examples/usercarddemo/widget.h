#pragma once
#include <QLabel>
#include <QWidget>

class Widget : public QWidget
{
    Q_OBJECT
public:
    explicit Widget(QWidget *parent = nullptr);
};
