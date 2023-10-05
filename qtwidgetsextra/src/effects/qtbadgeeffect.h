#pragma once
#include <QGraphicsEffect>
#include <QtWidgetsExtra>

class QTWIDGETSEXTRA_EXPORT QtBadgeEffect :
        public QGraphicsEffect
{
    Q_OBJECT
public:
    explicit QtBadgeEffect(QObject* parent = Q_NULLPTR);
    ~QtBadgeEffect();

    void setMaximumSize(const QSize& size);
    QSize maximumSize() const;

    void setMargins(const QMargins& margins);
    QMargins margins() const;

    void setFont(const QFont& font);
    QFont font() const;

    int counter() const;
    QString text() const;

    QPixmap icon() const;

    void setValue(const QVariant& value);
    QVariant value() const;

    void setAlignment(Qt::Alignment align);
    Qt::Alignment alignment() const;

    void draw(QPainter* painter) Q_DECL_OVERRIDE;

public Q_SLOTS:
    void setCounter(int value);
    void setIcon(const QPixmap& icon);

private:
    QScopedPointer<class QtBageEffectPrivate> d;
};
