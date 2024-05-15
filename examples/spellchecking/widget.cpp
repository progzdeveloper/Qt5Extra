#include "widget.h"
#include "spellchecksettings.h"
#include <QtSpellChecker>
#include <QtSpellCompleter>
#include <QtPluginManagerDialog>

Widget::Widget(QWidget* parent)
{
    pluginsButton = new QToolButton(this);
    pluginsButton->setIcon(QIcon::fromTheme("emblem-system"));
    pluginsButton->setText("...");
    pluginsButton->setToolTip("About Plugins");
    connect(pluginsButton, &QToolButton::clicked, this, &Widget::aboutPlugins);

    settingsButton = new QToolButton(this);
    settingsButton->setText("...");
    settingsButton->setToolTip("SpellCheck Settings");
    settingsButton->setIcon(QIcon::fromTheme("tools-check-spelling"));
    connect(settingsButton, &QToolButton::clicked, this, &Widget::showSettings);

    lineEdit = new QLineEdit(this);
    lineEdit->setAttribute(Qt::WA_Hover);

    QtSpellChecker* checker = new QtSpellChecker(lineEdit);
    checker->completer()->setOptions(QtSpellCompleter::SuggestsTooltip | QtSpellCompleter::ShortcutMenu | QtSpellCompleter::ContextMenuEmbed);
    checker->completer()->setShortcut(Qt::ControlModifier + Qt::Key_Space);
    textEdit = new QTextEdit(this);
    new QtSpellChecker(textEdit);

    QHBoxLayout* boxLayout = new QHBoxLayout;
    boxLayout->addWidget(lineEdit);
    boxLayout->addWidget(settingsButton);
    boxLayout->addWidget(pluginsButton);
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addLayout(boxLayout);
    layout->addWidget(textEdit);
}

void Widget::aboutPlugins()
{
    QtPluginManagerDialog dlg;
    dlg.exec();
}

void Widget::showSettings()
{
    SpellCheckSettings settingsDialog;
    settingsDialog.exec();
}
