#include "widget.h"
#include <QtSpellChecker>
#include <QtSpellCompleter>
#include <QtPluginManagerDialog>

Widget::Widget(QWidget* parent)
{
    button = new QToolButton(this);
    connect(button, &QToolButton::clicked, this, &Widget::aboutPlugins);
    lineEdit = new QLineEdit(this);
    lineEdit->setAttribute(Qt::WA_Hover);
    QtSpellChecker* checker = new QtSpellChecker(lineEdit);
    checker->completer()->setOptions(QtSpellCompleter::SuggestsTooltip | QtSpellCompleter::ShortcutMenu | QtSpellCompleter::ContextMenuEmbed);
    checker->completer()->setShortcut(Qt::ControlModifier + Qt::Key_Space);
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
