#include "qtsqlconnectionedit.h"

#include <QFormLayout>
#include <QBoxLayout>
#include <QCompleter>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QTableWidget>

#include <QSqlDatabase>
#include <QSqlError>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#ifdef QTSQLEXTRA_DLL
#include <QtSqlUtils>
#endif

#ifdef QTWIDGETSEXTRA_DLL
#include <QtLineBoxEdit>
#include <QtEditDialog>
#endif

#include <QDebug>

#include <QCoreApplication>

#include <QThread>

namespace
{

static const char *fieldNames[] = {
    QT_TRANSLATE_NOOP("QtSqlConnectionEditPrivate", "Connection"), // 0 - ConnectionName
    QT_TRANSLATE_NOOP("QtSqlConnectionEditPrivate", "Driver"),     // 1 - DriverName,
    QT_TRANSLATE_NOOP("QtSqlConnectionEditPrivate", "Database"),   // 2 - DatabaseName,
    QT_TRANSLATE_NOOP("QtSqlConnectionEditPrivate", "User"),       // 3 - UserName,
    QT_TRANSLATE_NOOP("QtSqlConnectionEditPrivate", "Password"),   // 4 - Password,
    QT_TRANSLATE_NOOP("QtSqlConnectionEditPrivate", "Host"),       // 5 - HostName,
    QT_TRANSLATE_NOOP("QtSqlConnectionEditPrivate", "Port"),       // 6 - Port,
    QT_TRANSLATE_NOOP("QtSqlConnectionEditPrivate", "Options")     // 7 - ConnectionOptions
};

static const char *fieldPlaceholders[] = {
    QT_TRANSLATE_NOOP("QtSqlConnectionEditPrivate", "Enter connection name.."),    // 0 - ConnectionName
    QT_TRANSLATE_NOOP("QtSqlConnectionEditPrivate", "Select driver..."),           // 1 - DriverName,
    QT_TRANSLATE_NOOP("QtSqlConnectionEditPrivate", "Enter database name..."),     // 2 - DatabaseName,
    QT_TRANSLATE_NOOP("QtSqlConnectionEditPrivate", "Enter user name..."),         // 3 - UserName,
    QT_TRANSLATE_NOOP("QtSqlConnectionEditPrivate", "Enter password..."),          // 4 - Password,
    QT_TRANSLATE_NOOP("QtSqlConnectionEditPrivate", "Enter host..."),              // 5 - HostName,
    QT_TRANSLATE_NOOP("QtSqlConnectionEditPrivate", "Enter port..."),              // 6 - Port,
    QT_TRANSLATE_NOOP("QtSqlConnectionEditPrivate", "Enter connection options...") // 7 - ConnectionOptions
};

}

class QtSqlConnectionEditPrivate
{
    Q_DECLARE_TR_FUNCTIONS(QtSqlConnectionEditPrivate)
public:
    QHash<QtSqlConnectionEdit::Field, QWidget*> editors;

    QJsonObject driverOptions;
    QtSqlConnectionEdit* q;
    QCheckBox* echoSwitcher;
    QFormLayout* formLayout;
    bool isReadOnly;

    QtSqlConnectionEditPrivate(QtSqlConnectionEdit* editor);

    void initUi();
    void createEditors();
    void setupPlaceholders();
    void setReadOnly(bool on);

    void setValue(int field, const QVariant& value);
    QVariant value(int field) const;

    void clear();
#ifdef QTWIDGETSEXTRA_DLL
    bool setupDialog(const QString &driverName, QtEditDialog* dialog);
    void setupOptions(QtEditDialog* dialog);
#endif
};

QtSqlConnectionEditPrivate::QtSqlConnectionEditPrivate(QtSqlConnectionEdit *editor) :
    q(editor), isReadOnly(false)
{
    QFile file(":/text/connect_options.json");
    if (!file.open(QFile::ReadOnly))
        return;

    driverOptions = QJsonDocument::fromJson(file.readAll()).object();
}

void QtSqlConnectionEditPrivate::initUi()
{
    createEditors();
    setupPlaceholders();
    formLayout = new QFormLayout(q);
    for (int i = 0; i <= QtSqlConnectionEdit::ConnectionOptions; i++) {
        formLayout->addRow(tr(fieldNames[i]) + ':', editors[static_cast<QtSqlConnectionEdit::Field>(i)]);
    }

    formLayout->addRow(echoSwitcher);
}

void QtSqlConnectionEditPrivate::createEditors()
{
    QLineEdit* connectionNameEdit = new QLineEdit(q);
    connectionNameEdit->setCompleter(new QCompleter(QStringList() << QSqlDatabase::connectionNames(), connectionNameEdit));

    QComboBox* driverNameEdit = new QComboBox(q);
    QStringList driversList = QSqlDatabase::drivers();
    for (auto it = driversList.begin(); it != driversList.end(); ++it) {
#ifdef QTSQLEXTRA_DLL
        driverNameEdit->addItem(QtSql::driverVendor(*it), *it);
#else
        driverNameEdit->addItem(*it, *it);
#endif
    }
    QObject::connect(driverNameEdit, &QComboBox::currentTextChanged, q, &QtSqlConnectionEdit::driverChanged);

    editors[QtSqlConnectionEdit::ConnectionName] = connectionNameEdit;
    editors[QtSqlConnectionEdit::DriverName] = driverNameEdit;
    editors[QtSqlConnectionEdit::DatabaseName] = new QLineEdit(q);
    editors[QtSqlConnectionEdit::UserName] = new QLineEdit(q);
    editors[QtSqlConnectionEdit::Password] = new QLineEdit(q);
    editors[QtSqlConnectionEdit::Password]->setProperty("echoMode", QLineEdit::Password);
    editors[QtSqlConnectionEdit::HostName] = new QLineEdit(q);
    editors[QtSqlConnectionEdit::Port] = new QSpinBox(q);
    editors[QtSqlConnectionEdit::Port]->setProperty("minimum", 0);
    editors[QtSqlConnectionEdit::Port]->setProperty("maximum", 65536);
#ifdef QTWIDGETSEXTRA_DLL
    editors[QtSqlConnectionEdit::ConnectionOptions] = new QtLineBoxEdit(q);
    QObject::connect(editors[QtSqlConnectionEdit::ConnectionOptions], &QtLineBoxEdit::clicked, q, &QtSqlConnectionEdit::editOptions);
#else
    editors[QtSqlConnectionEdit::ConnectionOptions] = new QPlainTextEdit(q);
    editors[QtSqlConnectionEdit::ConnectionOptions]->setMaximumHeight(100);
#endif
    echoSwitcher = new QCheckBox(tr("Show password"), q);
    echoSwitcher->setChecked(false);
    QObject::connect(echoSwitcher, &QCheckBox::toggled, q, &QtSqlConnectionEdit::enableEchoMode);
}

void QtSqlConnectionEditPrivate::setupPlaceholders()
{
    QLineEdit* lineEdit = Q_NULLPTR;
    for (auto it = editors.cbegin(); it != editors.cend(); ++it)
    {
        QWidget* editor = *it;
        lineEdit = qobject_cast<QLineEdit*>(editor);
        if (lineEdit == Q_NULLPTR)
            lineEdit = editor->findChild<QLineEdit*>();

        if (lineEdit != Q_NULLPTR) {
            lineEdit->setProperty("placeholderText", tr(fieldPlaceholders[it.key()]));
#if (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
            lineEdit->setProperty("clearButtonEnabled", true);
#endif
        }
    }
}

void QtSqlConnectionEditPrivate::setReadOnly(bool on)
{
    QLineEdit* lineEdit = Q_NULLPTR;
    for (auto it = editors.cbegin(); it != editors.cend(); ++it)
    {
        QWidget* editor = *it;
        lineEdit = qobject_cast<QLineEdit*>(editor);
        if (lineEdit == Q_NULLPTR)
            lineEdit = editor->findChild<QLineEdit*>();

        if (lineEdit != Q_NULLPTR) {
            lineEdit->setProperty("placeholderText", on ? QString() : tr(fieldPlaceholders[it.key()]));
            lineEdit->setProperty("readOnly", on);
        }
    }
    isReadOnly = on;
}

void QtSqlConnectionEditPrivate::setValue(int field, const QVariant &value)
{
    typedef QtSqlConnectionEdit::Field field_t;
    switch(field) {
    case QtSqlConnectionEdit::ConnectionName:
        editors[static_cast<field_t>(field)]->setProperty("text", value.toString());
        break;
    case QtSqlConnectionEdit::DriverName:
        editors[static_cast<field_t>(field)]->setProperty("currentText", value.toString());
        break;
    case QtSqlConnectionEdit::DatabaseName:
    case QtSqlConnectionEdit::UserName:
    case QtSqlConnectionEdit::Password:
    case QtSqlConnectionEdit::HostName:
        editors[static_cast<field_t>(field)]->setProperty("text", value.toString());
        break;
    case QtSqlConnectionEdit::Port:
        editors[static_cast<field_t>(field)]->setProperty("value", value.toInt());
        break;
    case QtSqlConnectionEdit::ConnectionOptions:
#ifdef QTWIDGETSEXTRA_DLL
        editors[static_cast<field_t>(field)]->setProperty("text", value.toString());
#else
        editors[static_cast<field_t>(field)]->setProperty("plainText", value.toString());
#endif
        break;
    default:
        break;
    }
}

QVariant QtSqlConnectionEditPrivate::value(int field) const
{
    typedef QtSqlConnectionEdit::Field field_t;
    switch(field) {
    case QtSqlConnectionEdit::ConnectionName:
        return editors[static_cast<field_t>(field)]->property("text");
    case QtSqlConnectionEdit::DriverName:
        return static_cast<QComboBox*>(editors[static_cast<field_t>(field)])->currentData().toString();
    case QtSqlConnectionEdit::DatabaseName:
    case QtSqlConnectionEdit::UserName:
    case QtSqlConnectionEdit::Password:
    case QtSqlConnectionEdit::HostName:
        return editors[static_cast<field_t>(field)]->property("text");
    case QtSqlConnectionEdit::Port:
        return editors[static_cast<field_t>(field)]->property("value");
    case QtSqlConnectionEdit::ConnectionOptions:
        return editors[static_cast<field_t>(field)]->property("plainText");
    default:
        break;
    }
    return QVariant();
}

void QtSqlConnectionEditPrivate::clear()
{
    QVariant value;
    for (int i = 0; i <= QtSqlConnectionEdit::ConnectionOptions; i++) {
        setValue(i, value);
    }
}

#ifdef QTWIDGETSEXTRA_DLL
bool QtSqlConnectionEditPrivate::setupDialog(const QString& driverName, QtEditDialog* editor)
{
    editor->registerEditor<QVariant::StringList, QComboBox>("currentText");
    QJsonObject json;
    auto it = driverOptions.constFind(driverName);
    if (it != driverOptions.end()) {
        json = it.value().toObject();
    } else {
        for (auto it = driverOptions.constBegin(); it != driverOptions.constEnd(); ++it)
        {
            if (driverName.contains(it.key())) {
                json = it.value().toObject();
                break;
            }
        }
    }

    if (json.isEmpty())
        return false;

    for (auto it = json.begin(); it != json.end(); ++it)
    {
        const QJsonObject attribute = it.value().toObject();
        const QString type = attribute["type"].toString();
        if (type == "QString")
            editor->insert(it.key(), it.key(), QVariant(QVariant::String), true);
        if (type == "UInt")
            editor->insert(it.key(), it.key(), (uint)attribute["default"].toInt(), true);
        if (type == "Int")
            editor->insert(it.key(), it.key(), attribute["default"].toInt(), true);
        if (type == "QStringList") {
            const QJsonArray array =  attribute["values"].toArray();
            QStringList keys;
            for (auto it = array.constBegin(); it != array.constEnd(); ++it) {
                keys << (*it).toString();
            }
            editor->insert(it.key(), it.key(), keys, true);
        }
    }
    return true;
}

void QtSqlConnectionEditPrivate::setupOptions(QtEditDialog *dialog)
{
    QString options;
    const QVariantHash values = dialog->values();
    for (auto it = values.cbegin(); it != values.cend(); ++it) {
        options += it.key();
        options += '=';
        options += it.value().toString();
        options += ';';
    }
    setValue(QtSqlConnectionEdit::ConnectionOptions, options);
    editors[QtSqlConnectionEdit::ConnectionOptions]->setToolTip(options.replace(";", ";\n"));
}
#endif


QtSqlConnectionEdit::QtSqlConnectionEdit(QWidget *parent, Qt::WindowFlags flags) :
    QWidget(parent, flags), d(new QtSqlConnectionEditPrivate(this))
{
    d->initUi();
    //adjustSize();
    //setFixedHeight(height());
}

QtSqlConnectionEdit::~QtSqlConnectionEdit()
{
}

void QtSqlConnectionEdit::setDatabase(const QSqlDatabase &db)
{
    setConnectionName(db.connectionName());
    setDriverName(db.driverName());
    setDatabaseName(db.databaseName());
    setUserName(db.userName());
    setPassword(db.password());
    setHostName(db.hostName());
    setPort(db.port());
    setOptions(db.connectOptions());
}

void QtSqlConnectionEdit::setReadOnly(bool on)
{
    if (on != d->isReadOnly)
        d->setReadOnly(on);
}

bool QtSqlConnectionEdit::isReadOnly() const
{
    return d->isReadOnly;
}

void QtSqlConnectionEdit::setConnectionName(const QString &name)
{
    d->setValue(ConnectionName, name);
}

QString QtSqlConnectionEdit::connectionName() const
{
    return d->value(ConnectionName).toString();
}

void QtSqlConnectionEdit::setDatabaseName(const QString &databaseName)
{
    d->setValue(DatabaseName, databaseName);
}

QString QtSqlConnectionEdit::databaseName() const
{
    return d->value(DatabaseName).toString();
}

void QtSqlConnectionEdit::setDriverName(const QString &driverName)
{
    d->setValue(DriverName, driverName);
}

QString QtSqlConnectionEdit::driverName() const
{
    return d->value(DriverName).toString();
}

void QtSqlConnectionEdit::setUserName(const QString &userName)
{
    d->setValue(UserName, userName);
}

QString QtSqlConnectionEdit::userName() const
{
    return d->value(UserName).toString();
}

void QtSqlConnectionEdit::setPassword(const QString &password)
{
    d->setValue(Password, password);
}

QString QtSqlConnectionEdit::password() const
{
    return d->value(Password).toString();
}

void QtSqlConnectionEdit::setHostName(const QString& hostName)
{
    d->setValue(HostName, hostName);
}

QString QtSqlConnectionEdit::hostName() const
{
    return d->value(HostName).toString();
}

void QtSqlConnectionEdit::setPort(int port)
{
    d->setValue(Port, port);
}

int QtSqlConnectionEdit::port() const
{
    return d->value(Port).toInt();
}

void QtSqlConnectionEdit::setOptions(const QString& options)
{
    d->setValue(ConnectionOptions, options);
}


QString QtSqlConnectionEdit::options() const
{
    return d->value(ConnectionOptions).toString();
}

bool QtSqlConnectionEdit::verifyInput()
{
    QWidget* editor = Q_NULLPTR;

    editor = d->editors[QtSqlConnectionEdit::DriverName];
    if (editor->property("currentText").value<QString>().isEmpty())
    {
        Q_EMIT reportWarning(tr("No SQL driver was selected.\nPlease, select SQL driver and try again."));
        editor->setFocus();
        return false;
    }

    editor = d->editors[QtSqlConnectionEdit::ConnectionName];
    if (editor->property("text").value<QString>().isEmpty())
    {
        Q_EMIT reportWarning(tr("Connection name was not specified.\nSpecify connection name and try again."));
        editor->setFocus();
        return false;
    }
    return true;
}



void QtSqlConnectionEdit::enableEchoMode(bool on)
{
    d->editors[Password]->setProperty("echoMode", on ? QLineEdit::Normal : QLineEdit::Password);
}

void QtSqlConnectionEdit::editOptions()
{
#ifdef QTWIDGETSEXTRA_DLL
    QString name = driverName();
    QtEditDialog dialog;
    if (!d->setupDialog(name, &dialog))
        return;

    dialog.setWindowTitle(tr("Connection options"));
    if (dialog.exec() == QDialog::Rejected)
        return;
    d->setupOptions(&dialog);
#endif
}

void QtSqlConnectionEdit::driverChanged(const QString &)
{
    d->setValue(ConnectionOptions, QVariant(QVariant::String));
}

void QtSqlConnectionEdit::test()
{
    if (!verifyInput())
        return;

    QString key = connectionName();
    // check connection pool first
    if (QSqlDatabase::contains(key))
    {
        QSqlDatabase database = QSqlDatabase::database(key, false);
        bool wasOpen = database.isOpen();
        if (wasOpen) {
            Q_EMIT testSucceeded();
            return;
        }

        bool result = database.open();
        if (result) {
            Q_EMIT testSucceeded();
            database.close();
        } else {
            Q_EMIT testFailed(database.lastError().text());
        }
    }
    else
    {
        {
            QSqlDatabase database = QSqlDatabase::addDatabase(driverName(), key);
            database.setDatabaseName(databaseName());
            database.setUserName(userName());
            database.setPassword(password());
            database.setHostName(hostName());
            database.setPort(port());
            database.setConnectOptions(options());

            bool result = database.open();
            if (result) {
                Q_EMIT testSucceeded();
                database.close();
            } else {
                Q_EMIT testFailed(database.lastError().text());
            }
        }
        QSqlDatabase::removeDatabase(key);
    }
}
