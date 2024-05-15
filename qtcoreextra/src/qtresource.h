#pragma once
#include <QtCore/QResource>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QtCoreExtra>

class QBuffer;

class QTCOREEXTRA_EXPORT QtResource :
        public QResource
{
public:
    QtResource(const QString &file = QString(), const QLocale &locale = QLocale());

    QString toString() const;
    QByteArray toByteArray() const;
    QStringList toStringList() const;
    QJsonDocument toJsonDocument(QJsonParseError* error = nullptr) const;

    void buffer(QBuffer& buffer) const;
};
