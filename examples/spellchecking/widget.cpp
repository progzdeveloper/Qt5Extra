#include "widget.h"
#include <QtSpellChecker>
#include <QtPluginManagerDialog>

Widget::Widget(QWidget* parent)
{
    button = new QToolButton(this);
    connect(button, &QToolButton::clicked, this, &Widget::aboutPlugins);
    lineEdit = new QLineEdit(this);
    new QtSpellChecker(lineEdit);

    textEdit = new QTextEdit(this);
    new QtSpellChecker(textEdit);

    QHBoxLayout* boxLayout = new QHBoxLayout;
    boxLayout->addWidget(lineEdit);
    boxLayout->addWidget(button);
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addLayout(boxLayout);
    layout->addWidget(textEdit);
}

void Widget::aboutPlugins()
{
    QtPluginManagerDialog dlg;
    dlg.exec();
}
