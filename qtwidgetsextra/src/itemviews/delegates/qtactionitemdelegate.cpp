#include "qtactionitemdelegate.h"
#include <QAction>
#include <QToolButton>
#include <QHBoxLayout>
#include <QAbstractItemView>
#include <QSignalMapper>
#include <QApplication>

#include <QDebug>

#include <QtItemModelUtility>


class QtActionItemDelegatePrivate
{
public:
    QList<QAction*> actions;
    Qt::ToolButtonStyle buttonStyle;
    bool isAutoRaise;
    QSize iconSize;
    QSignalMapper* mapper;

    QToolButton* createButton(QAction* act, QWidget* parent) const;
    void fillLayout(QHBoxLayout* layout, int side, QWidget* parent) const;
};

QToolButton* QtActionItemDelegatePrivate::createButton(QAction* act, QWidget* parent) const
{
    QToolButton* button = new QToolButton(parent);
    button->setToolButtonStyle(buttonStyle);
    button->setAutoRaise(isAutoRaise);
    button->setIconSize(iconSize);
    button->setIcon(act->icon());
    button->setText(act->text());
    button->setToolTip(act->toolTip());
    button->setStatusTip(act->statusTip());
    button->setEnabled(act->isEnabled());
    button->setVisible(act->isVisible());
    button->adjustSize();
    button->setFixedSize(button->size());
    QObject::connect(button, &QToolButton::clicked, act, &QAction::triggered);
    return button;
}

void QtActionItemDelegatePrivate::fillLayout(QHBoxLayout* layout, int side, QWidget* parent) const
{ 
    QList<QAction*>::const_iterator it = actions.begin();
    for (; it != actions.end(); ++it) {
        if ((*it)->property("side").toInt() == side) {
            QToolButton* button = createButton(*it, parent);
            QObject::connect(*it, &QAction::changed, mapper, qOverload<>(&QSignalMapper::map));
            mapper->setMapping(*it, button);
            layout->addWidget(button);
        }
    }
}



QtActionItemDelegate::QtActionItemDelegate(QObject* parent /* = 0*/) :
    QStyledItemDelegate(parent),
    d(new QtActionItemDelegatePrivate)
{
    d->buttonStyle = Qt::ToolButtonFollowStyle;
    d->isAutoRaise = true;
    int s = qApp->style()->pixelMetric(QStyle::PM_ButtonIconSize);
    d->iconSize = QSize(s, s);
    d->mapper = new QSignalMapper(this);
};

QtActionItemDelegate::~QtActionItemDelegate() = default;

void QtActionItemDelegate::setToolButtonStyle(Qt::ToolButtonStyle style)
{
    d->buttonStyle = style;
}

Qt::ToolButtonStyle QtActionItemDelegate::toolButtonStyle() const
{
    return d->buttonStyle;
}

void QtActionItemDelegate::setAutoRaise(bool on)
{
    d->isAutoRaise = on;
}

bool QtActionItemDelegate::isAutoRaise() const
{
    return d->isAutoRaise;
}

void QtActionItemDelegate::setIconSize(const QSize& size)
{
    d->iconSize = size;
}

QSize QtActionItemDelegate::iconSize() const
{
    return d->iconSize;
}

void QtActionItemDelegate::addActions(const QList<QAction*>& actions, Side side /*= RightSide*/)
{
    for (QList<QAction*>::const_iterator it = actions.begin(); it != actions.end(); ++it) {
        addAction(*it, side);
    }
}

void QtActionItemDelegate::addAction(QAction* action, Side side /*= RightSide*/)
{
    action->setProperty("side", side);
    d->actions << action;
}

void QtActionItemDelegate::removeAction(QAction* action)
{
    d->actions.removeAll(action);
    d->mapper->removeMappings(action);
    QObject::disconnect(action, nullptr, d->mapper, nullptr);
}

QList<QAction*> QtActionItemDelegate::actions() const
{
    return d->actions;
}

QWidget *QtActionItemDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    QWidget* editor = QStyledItemDelegate::createEditor(parent, option, index);

    QtItemEditor* itemEditor = new QtItemEditor(editor, parent);

    QHBoxLayout* layout = new QHBoxLayout(itemEditor);
    layout->setMargin(0);
    layout->setSpacing(3);

    d->fillLayout(layout, LeftSide, itemEditor);
    layout->addWidget( editor );
    layout->addStretch();
    d->fillLayout(layout, RightSide, itemEditor);

    connect(d->mapper, qOverload<QWidget*>(&QSignalMapper::mapped), this, &QtActionItemDelegate::updateButton);
    connect(itemEditor, &QObject::destroyed, this, &QtActionItemDelegate::editorDestroyed);

    return itemEditor;
}

void QtActionItemDelegate::setEditorData(QWidget * editor, const QModelIndex & index) const
{
    QtItemEditor* itemEditor = qobject_cast<QtItemEditor*>(editor);
    if (itemEditor) {
        QStyledItemDelegate::setEditorData(itemEditor->widget(), index);
    } else {
        QStyledItemDelegate::setEditorData(editor, index);
    }
}

void QtActionItemDelegate::setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const
{
    QtItemEditor* itemEditor = qobject_cast<QtItemEditor*>(editor);
    if (itemEditor) {
        QStyledItemDelegate::setModelData(itemEditor->widget(), model, index);
    } else {
        QStyledItemDelegate::setModelData(editor, model, index);
    }
}


void QtActionItemDelegate::updateButton(QWidget* widget)
{
    QToolButton* button = qobject_cast<QToolButton*>(widget);
    if (!button)
        return;

    QAction* act = qobject_cast<QAction*>(d->mapper->mapping(widget));
    if (!act)
        return;

    button->setIcon(act->icon());
    button->setText(act->text());
    button->setToolTip(act->toolTip());
    button->setStatusTip(act->statusTip());
    button->setEnabled(act->isEnabled());
    button->setVisible(act->isVisible());
}

void QtActionItemDelegate::updateEditorGeometry(QWidget * editor, const QStyleOptionViewItem & option, const QModelIndex &/* index*/) const
{
    editor->setGeometry(option.rect);
}

void QtActionItemDelegate::editorDestroyed(QObject*)
{
    QList<QAction*>::const_iterator it = d->actions.cbegin();
    for (; it != d->actions.cend(); ++it)
    {
        d->mapper->removeMappings(*it);
        QObject::disconnect(*it, nullptr, d->mapper, nullptr);
    }
}

