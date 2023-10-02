#include "qteditdialog.h"

#include <QMetaObject>
#include <QMetaProperty>
#include <QMetaMethod>

#include <QDialogButtonBox>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>

#include <QFormLayout>
#include <QVBoxLayout>

#include <QItemEditorFactory>
#include <QItemEditorCreatorBase>
#include "../itemviews/delegates/qtvariantitemdelegate.h"

namespace
{

static QtVariantItemEditorFactory& itemFactory() {
    static QtVariantItemEditorFactory instance;
    return instance;
}

}

struct QtValueAttribute
{
    QString title;
    QVariant value;
    bool checkable;

    QtValueAttribute() : checkable(false) {}

    QtValueAttribute(QString aTitle,QVariant aValue,bool isCheckable) :
        title(aTitle), value(aValue), checkable(isCheckable) {}
};

class QtEditDialogPrivate
{
public:
    QHash<QString, QWidget*> labels;
    QHash<QString, QWidget*> editors;
    QHash<QString, QByteArray> propertyMap;
    QHash<QString, QtValueAttribute> attributeMap;
    QItemEditorFactory* editorFactory;
    QDialogButtonBox* buttonBox;
    QFormLayout* formLayout;
    QVBoxLayout* mainLayout;
    QtEditDialog* q_ptr;
    bool isCheckable;

    QtEditDialogPrivate(QtEditDialog* q, QItemEditorFactory *factory);
    void initialize();

    void createEditor(const QString &objectName,
                          const QString &title,
                          const QVariant &value,
                          bool checkable);
    void updateUi();
};

QtEditDialogPrivate::QtEditDialogPrivate(QtEditDialog *q, QItemEditorFactory* factory)
    : editorFactory(factory)
    , q_ptr(q)
    , isCheckable(false)
{
}

void QtEditDialogPrivate::initialize()
{
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel, Qt::Horizontal, q_ptr);
    QObject::connect(buttonBox, &QDialogButtonBox::accepted, q_ptr, &QtEditDialog::accept);
    QObject::connect(buttonBox, &QDialogButtonBox::rejected, q_ptr, &QtEditDialog::reject);

    formLayout = new QFormLayout;
    mainLayout = new QVBoxLayout(q_ptr);
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(buttonBox);
}

void QtEditDialogPrivate::createEditor(const QString &objectName, const QString &title, const QVariant &value, bool checkable)
{
    QByteArray propertyName = editorFactory->valuePropertyName(value.type());
    auto it = editors.constFind(objectName);
    if (it == editors.cend())
    {
        QWidget* editor = editorFactory->createEditor(value.type(), q_ptr);
        int propertyIndex = editor->metaObject()->indexOfProperty(propertyName);
        const QMetaProperty metaProperty = editor->metaObject()->property(propertyIndex);
        if (metaProperty.hasNotifySignal()) {
            int methodIndex = q_ptr->metaObject()->indexOfSlot(QMetaObject::normalizedSignature("editorValueChanged()"));
            const QMetaMethod method = q_ptr->metaObject()->method(methodIndex);
            QObject::connect(editor, metaProperty.notifySignal(), q_ptr, method);
        }
        editor->setObjectName(objectName);

        propertyMap[objectName] = propertyName;

        it = editors.insert(objectName, editor);
        QWidget* label = Q_NULLPTR;
        if (checkable) {
            QCheckBox* checkBox = new QCheckBox(q_ptr);
            checkBox->setCheckable(checkable);
            checkBox->setChecked(true);
            checkBox->setText(title);
            label = checkBox;
        } else {
            label = new QLabel(title, q_ptr);
        }
        labels[objectName] = label;
        formLayout->addRow(label, editor);
    }

    if (value.type() == QVariant::StringList ||
        value.type() == QVariant::List) {
        QComboBox* combo = qobject_cast<QComboBox*>(*it);
        if (combo) {
            combo->clear();
            combo->addItems(value.toStringList());
        }
    }

    (*it)->setProperty(propertyName, value);
}

void QtEditDialogPrivate::updateUi()
{
    while(formLayout->rowCount() > 0) {
        formLayout->removeRow(formLayout->rowCount() - 1);
    }
    editors.clear();
    labels.clear();
    propertyMap.clear();
    for (auto it = attributeMap.cbegin(); it != attributeMap.cend(); ++it)
        createEditor(it.key(), it->title, it->value, it->checkable);
}


QtEditDialog::QtEditDialog(QWidget *parent)
    : QtEditDialog(&itemFactory(), parent)
{
}

QtEditDialog::QtEditDialog(QItemEditorFactory *factory, QWidget *parent)
    : QDialog(parent)
    , d(new QtEditDialogPrivate(this, factory))
{
    d->initialize();
}

QtEditDialog::~QtEditDialog() = default;

void QtEditDialog::registerEditor(int userType, QItemEditorCreatorBase *creator)
{
     
    d->editorFactory->registerEditor(userType, creator);
}

bool QtEditDialog::isChecked(const QString &objectName) const
{
     
    auto it = d->labels.constFind(objectName);
    if (it == d->labels.cend())
        return false;

    QCheckBox* checkBox = qobject_cast<QCheckBox*>(*it);
    return (checkBox != Q_NULLPTR ? checkBox->isChecked() : false);
}

void QtEditDialog::insert(const QString &title, const QString &objectName, const QVariant &value, bool checkable)
{
     
    d->attributeMap[objectName] = QtValueAttribute(title, value, checkable);
    d->createEditor(objectName, title, value, checkable);
}

QVariantHash QtEditDialog::values() const
{
     
    QVariantHash results;
    for (auto it = d->editors.cbegin(); it != d->editors.cend(); ++it)
    {
        if (isChecked(it.key())) {
            results[it.key()] = (*it)->property(d->propertyMap[it.key()]);
        }
    }
    return results;
}

QVariant QtEditDialog::value(const QString &id) const
{
     
    auto it = d->editors.constFind(id);
    return (it != d->editors.cend() ? (*it)->property(d->propertyMap[it.key()]) : QVariant());
}

void QtEditDialog::editorValueChanged()
{
     
    QObject* editor = sender();
    auto it = d->propertyMap.constFind(editor->objectName());
    if (it != d->propertyMap.cend())
        Q_EMIT valueChanged(editor->objectName(), editor->property(*it));
}

