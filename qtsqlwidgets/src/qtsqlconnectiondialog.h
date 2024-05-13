#pragma once
#include <QDialog>
#include <QtSqlWidgets>

class QTSQLWIDGETS_EXPORT QtSqlConnectionDialog : public QDialog
{
    Q_OBJECT

    Q_PROPERTY(QString connectionName READ connectionName WRITE setConnectionName)
    Q_PROPERTY(QString databaseName READ databaseName WRITE setDatabaseName)
    Q_PROPERTY(QString driverName READ driverName WRITE setDriverName)
    Q_PROPERTY(QString userName READ userName WRITE setUserName)
    Q_PROPERTY(QString password READ password WRITE setPassword)
    Q_PROPERTY(QString hostName READ hostName WRITE setHostName)
    Q_PROPERTY(QString options READ options WRITE setOptions)
    Q_PROPERTY(bool messagesEnabled READ isMessagesEnabled WRITE setMessagesEnabled)

public:
    QtSqlConnectionDialog(QWidget* parent = Q_NULLPTR, Qt::WindowFlags flags = 0);
    ~QtSqlConnectionDialog();

    QString connectionName() const;
    QString databaseName() const;
    QString driverName() const;
    QString userName() const;
    QString password() const;
    QString hostName() const;
    int port() const;
    QString options() const;
    bool isMessagesEnabled() const;

public Q_SLOTS:
    void setConnectionName(const QString& name);
    void setDatabaseName(const QString& databaseName);
    void setDriverName(const QString& driverName);
    void setUserName(const QString& userName);
    void setPassword(const QString& password);
    void setHostName(const QString& hostName);
    void setPort(int port);
    void setOptions(const QString& options);
    void setMessagesEnabled(bool on);

    void test();

private Q_SLOTS:
    void success();
    void fail(const QString& message);
    void warning(const QString& message);
    void error(const QString& message);

Q_SIGNALS:
    void testStarted();
    void testSucceeded();
    void testFailed(const QString& message);

private:
    friend class QtSqlConnectionDialogPrivate;
    QScopedPointer<class QtSqlConnectionDialogPrivate> d;
};
