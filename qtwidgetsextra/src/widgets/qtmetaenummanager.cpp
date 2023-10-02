#include <QMetaEnum>
#include <QVBoxLayout>
#include <QAction>
#include <QActionGroup>
#include <QComboBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QGroupBox>
#include <QIcon>

#include <QtEnumListModel>

#include "qtmetaenummanager.h"

QString QtAbstractMetaEnumManager::text(const QMetaEnum &metaEnum, int value) const
{
    return metaEnum.valueToKey(value);
}

QString QtAbstractMetaEnumManager::tooltip(const QMetaEnum &metaEnum, int value) const
{
    return text(metaEnum, value);
}

QIcon QtAbstractMetaEnumManager::icon(const QMetaEnum &metaEnum, int value) const
{
    return {};
}

QByteArray QtAbstractMetaEnumManager::editorProperty(const QMetaEnum&, int) const
{
    return "checked";
}

QWidget *QtAbstractMetaEnumManager::createValueEditor(const QMetaEnum &metaEnum, int value, QWidget *parent) const
{
    if (metaEnum.isFlag())
        return new QCheckBox(text(metaEnum, value), parent);
    else
        return new QRadioButton(text(metaEnum, value), parent);
}

QWidget *QtAbstractMetaEnumManager::createEnumEditor(const QMetaEnum &metaEnum, QWidget *parent) const
{
    QGroupBox* groupBox = new QGroupBox(parent);
    QVBoxLayout* layout = new QVBoxLayout(groupBox);
    for (int i = 0, n = metaEnum.keyCount(); i < n; ++i)
        layout->addWidget(createValueEditor(metaEnum, metaEnum.value(i), groupBox));
    return groupBox;
}

QAction *QtAbstractMetaEnumManager::createAction(const QMetaEnum &metaEnum, int value, QObject *parent) const
{
    QAction* action = new QAction(parent);
    action->setIcon(icon(metaEnum, value));
    action->setText(text(metaEnum, value));
    action->setToolTip(tooltip(metaEnum, value));
    action->setCheckable(metaEnum.isFlag());
    return action;
}

QActionGroup *QtAbstractMetaEnumManager::createActionGroup(const QMetaEnum &metaEnum, QObject *parent) const
{
    QActionGroup* actionGroup = new QActionGroup(parent);
    actionGroup->setExclusive(!metaEnum.isFlag());
    for (int i = 0, n = metaEnum.keyCount(); i < n; ++i)
    {
        const int value = metaEnum.value(i);
        QAction* action = createAction(metaEnum, value, actionGroup);
        action->setData(value);
        actionGroup->addAction(action);
    }
    return actionGroup;
}

QAbstractItemModel *QtAbstractMetaEnumManager::createItemModel(const QMetaEnum &metaEnum, QObject *parent) const
{
    return new QtEnumListModel(metaEnum, parent);
}
