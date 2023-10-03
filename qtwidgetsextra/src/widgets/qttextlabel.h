#pragma once
#include <QFrame>
#include <QtWidgetsExtra>

class QTWIDGETSEXTRA_EXPORT QtTextLabel
    : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY(bool isElided READ isElided)
public:
    explicit QtTextLabel(QWidget* parent = nullptr);
    explicit QtTextLabel(const QString& text, QWidget* parent = nullptr);
    ~QtTextLabel();

    void setText(const QString& text);
    QString text() const;

    QString elidedText() const;
    bool isElided() const;

Q_SIGNALS:
    void textChanged(const QString&);
    void elisionChanged(bool);

protected:
    void paintEvent(QPaintEvent* event);
    void resizeEvent(QResizeEvent* event);

private:
    QScopedPointer<class QtTextLabelPrivate> d;
};

