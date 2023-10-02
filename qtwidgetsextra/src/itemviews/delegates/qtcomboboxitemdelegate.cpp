#include "qtcomboboxitemdelegate.h"
#include <QComboBox>
#include <QCompleter>
#include <QValidator>
#include <QModelIndex>
#include <QStringList>

class QtComboBoxItemDelegatePrivate
{
public:
    QtComboBoxItemDelegate *q_ptr;
    QAbstractItemModel *model;
    QCompleter *completer;
    const QValidator *validator;
    int modelColumn;
    QModelIndex rootIndex;
    QStringList items;
};



QtComboBoxItemDelegate::QtComboBoxItemDelegate(QObject *parent) 
    : QtComboBoxItemDelegate({}, parent)
{

}

QtComboBoxItemDelegate::QtComboBoxItemDelegate(const QStringList &items, QObject *parent)
    : QItemDelegate(parent)
    , d(new QtComboBoxItemDelegatePrivate)
{
    d->q_ptr = this;
    d->modelColumn = 0;
    d->model = 0;
    d->completer = 0;
    d->validator = 0;
    addItems(items);
}

QtComboBoxItemDelegate::~QtComboBoxItemDelegate() = default;

void QtComboBoxItemDelegate::addItems(const QStringList& items)
{
    d->items << items;
}

void QtComboBoxItemDelegate::setItems(const QStringList& items)
{
    clear();
    addItems(items);
}

void QtComboBoxItemDelegate::clear()
{
    d->items.clear();
}


void QtComboBoxItemDelegate::setModel(QAbstractItemModel *model)
{
    d->model = model;
}

QAbstractItemModel * QtComboBoxItemDelegate::model() const
{
    return d->model;
}

void QtComboBoxItemDelegate::setModelColumn(int visibleColumn)
{
    d->modelColumn = visibleColumn;
}

int QtComboBoxItemDelegate::modelColumn() const
{
    return d->modelColumn;
}

void QtComboBoxItemDelegate::setRootModelIndex(const QModelIndex& index)
{
    d->rootIndex = index;
}

QModelIndex QtComboBoxItemDelegate::rootModelIndex () const
{
    return d->rootIndex;
}

void QtComboBoxItemDelegate::setValidator(const QValidator* validator)
{
    d->validator = validator;
}

const QValidator *QtComboBoxItemDelegate::validator () const
{
    return d->validator;
}

void QtComboBoxItemDelegate::setCompleter(QCompleter *completer)
{
    d->completer = completer;
}


QCompleter *QtComboBoxItemDelegate::completer() const
{
    return d->completer;
}

QWidget *QtComboBoxItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/*option*/,
                                              const QModelIndex &/*index*/) const
{
    QComboBox *editor = new QComboBox(parent);
    if (d->model) {
        editor->setModel(d->model);
        editor->setModelColumn(d->modelColumn);
    }
    else {
        editor->addItems(d->items);
    }
    editor->setCompleter(d->completer);
    editor->setValidator(d->validator);
    return editor;
}

void QtComboBoxItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QString value = index.data(Qt::EditRole).toString();

    QComboBox *comboBox = qobject_cast<QComboBox*>(editor);
    if (comboBox)
        comboBox->setCurrentIndex(comboBox->findText(value));
}

void QtComboBoxItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                          const QModelIndex &index) const
{
    QComboBox *comboBox = qobject_cast<QComboBox*>(editor);
    if (comboBox) {
        QString value = comboBox->currentText();
        model->setData(index, value, Qt::EditRole);
    }
}

void QtComboBoxItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, 
                                                  const QModelIndex &/*index*/) const
{
    editor->setGeometry(option.rect);
}



