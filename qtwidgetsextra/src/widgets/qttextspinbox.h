#pragma once
#include <QSpinBox>
#include <QtWidgetsExtra>

class QTWIDGETSEXTRA_EXPORT QtTextSpinBox :
        public QSpinBox
{
    Q_OBJECT
    Q_PROPERTY(QStringList list READ list WRITE setList)

public:
    explicit QtTextSpinBox(QWidget* parent = Q_NULLPTR);
    QtTextSpinBox(const QStringList &list, QWidget *parent = Q_NULLPTR);
    ~QtTextSpinBox();

    QValidator::State validate(QString& input, int& pos ) const Q_DECL_OVERRIDE;

    const QStringList &list() const;
public Q_SLOTS:
    virtual void setList(const QStringList &s);

Q_SIGNALS:
    void listChanged(const QStringList &);

protected:
    QString textFromValue(int value) const Q_DECL_OVERRIDE;
    int valueFromText(const QString & text) const Q_DECL_OVERRIDE;

private:
    QScopedPointer<class QtTextSpinBoxPrivate> d;
};
