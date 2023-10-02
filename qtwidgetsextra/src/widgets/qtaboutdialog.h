#pragma once
#include <QDialog>
#include <QtWidgetsExtra>

class QJsonDocument;

class QTWIDGETSEXTRA_EXPORT QtAboutDialog :
        public QDialog
{
    Q_OBJECT
    Q_DISABLE_COPY(QtAboutDialog)
public:
    explicit QtAboutDialog(QWidget *parent = Q_NULLPTR);
    explicit QtAboutDialog(const QJsonDocument& json, QWidget *parent = Q_NULLPTR);
    ~QtAboutDialog();

    void setJson(const QJsonDocument& document);
    void setJson(const QJsonObject& json);
    void setAboutText(const QString& text);
    void setVersionInfo(const QString& version);
    void setCopyrightText(const QString& text);
    void setLicenseText(const QString& text);
    void setLicenseFile(const QString& fileName);
    void setImage(const QPixmap& pixmap);

private:
    QScopedPointer<class QtAboutDialogPrivate> d;
};

