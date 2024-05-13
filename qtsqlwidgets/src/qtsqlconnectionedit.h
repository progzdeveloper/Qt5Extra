#pragma once
#include <QWidget>
#include <QtSqlWidgets>

class QSqlDatabase;

class QTSQLWIDGETS_EXPORT QtSqlConnectionEdit : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(QString connectionName READ connectionName WRITE setConnectionName)
    Q_PROPERTY(QString databaseName READ databaseName WRITE setDatabaseName)
    Q_PROPERTY(QString driverName READ driverName WRITE setDriverName)
    Q_PROPERTY(QString userName READ userName WRITE setUserName)
    Q_PROPERTY(QString password READ password WRITE setPassword)
    Q_PROPERTY(QString hostName READ hostName WRITE setHostName)
    Q_PROPERTY(int port READ port WRITE setPort)
    Q_PROPERTY(QString options READ options WRITE setOptions)
    Q_PROPERTY(bool isReadOnly READ isReadOnly)

public:
    enum Field {
        ConnectionName = 0,
        DriverName,
        DatabaseName,
        UserName,
        Password,
        HostName,
        Port,
        ConnectionOptions
    };

    explicit QtSqlConnectionEdit(QWidget *parent = Q_NULLPTR, Qt::WindowFlags flags = 0);
    ~QtSqlConnectionEdit();

    void setDatabase(const QSqlDatabase& db);

    bool isReadOnly() const;

    QString connectionName() const;
    QString databaseName() const;
    QString driverName() const;
    QString userName() const;
    QString password() const;
    QString hostName() const;
    int port() const;
    QString options() const;

    QVariant value(Field field);

    bool verifyInput();

public Q_SLOTS:
    void setReadOnly(bool on = true);

    void setConnectionName(const QString& name);
    void setDatabaseName(const QString& databaseName);
    void setDriverName(const QString& driverName);
    void setUserName(const QString& userName);
    void setPassword(const QString& password);
    void setHostName(const QString& hostName);
    void setPort(int port);
    void setOptions(const QString& options);

    void test();

private Q_SLOTS:
    void enableEchoMode(bool on);
    void editOptions();
    void driverChanged(const QString&);

Q_SIGNALS:
    void testFailed(const QString& message);
    void testSucceeded();
    void reportError(const QString& message);
    void reportWarning(const QString& message);

private:
    friend class QtSqlConnectionEditPrivate;
    QScopedPointer<class QtSqlConnectionEditPrivate> d;
};
