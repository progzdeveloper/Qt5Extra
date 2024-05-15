#include "qtresource.h"
#include <QtCore/QBuffer>
#include <QtCore/QTextStream>

QtResource::QtResource(const QString &file, const QLocale &locale) :
    QResource(file, locale)
{
}

QString QtResource::toString() const
{
    return QString::fromUtf8(reinterpret_cast<const char*>(this->data()), this->size());
}

QByteArray QtResource::toByteArray() const
{
    QByteArray ba;
    ba.setRawData(reinterpret_cast<const char*>(this->data()), this->size());
    return ba;
}

QStringList QtResource::toStringList() const
{
    QStringList strings;
    QBuffer buf;
    buffer(buf);
    buf.open(QBuffer::ReadOnly);
    QTextStream stream(&buf);
    QString line;
    while(!stream.atEnd()) {
        stream.readLineInto(&line);
        strings << line;
    }
    return strings;
}

QJsonDocument QtResource::toJsonDocument(QJsonParseError *error) const
{
    return QJsonDocument::fromJson(toByteArray(), error);
}

void QtResource::buffer(QBuffer& buffer) const
{
    buffer.setData(reinterpret_cast<const char*>(this->data()), this->size());
}



