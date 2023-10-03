#pragma once
#include <QStyle>
#include <QTextOption>
#include <QGraphicsEffect>
#include <QtWidgetsExtra>


class QTWIDGETSEXTRA_EXPORT QtGraphicsBadgeEffect :
        public QGraphicsEffect
{
    Q_OBJECT
public:
    explicit QtGraphicsBadgeEffect(QObject* parent = Q_NULLPTR);
    ~QtGraphicsBadgeEffect();

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
    QScopedPointer<class QtGraphicsBageEffectPrivate> d;
};


#include <QFrame>

class QtCardWidget :
    public QFrame
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY(QString comment READ comment WRITE setComment NOTIFY commentChanged)

public:
    explicit QtCardWidget(QWidget *parent = Q_NULLPTR);
    ~QtCardWidget();

    void setBadgeValue(const QVariant& value);
    QVariant badgeValue() const;

    QtGraphicsBadgeEffect& badge();

    void setCardStyle(Qt::ToolButtonStyle style);
    Qt::ToolButtonStyle cardStyle() const;

    // factor < 0 -> Circle
    // factor > 0 -> Rounded
    // factor = 0 -> Rect
    void setAvatarRoundness(int factor);
    int avatarRoundness() const;

    void setTextElideMode(Qt::TextElideMode mode);
    Qt::TextElideMode textElideMode() const;

    void setTextAlignment(Qt::Alignment align);
    Qt::Alignment textAlignment() const;

    void setCommentElideMode(Qt::TextElideMode mode);
    Qt::TextElideMode commentElideMode() const;

    void setCommentAlignment(Qt::Alignment align);
    Qt::Alignment commentAlignment() const;

    void setCommentWordWrap(bool on);
    bool isCommentWordWrap() const;

    QPixmap avatar() const;
    QString text() const;
    QString comment() const;

Q_SIGNALS:
    void textChanged(const QString&);
    void commentChanged(const QString&);
    void avatarChanged(const QPixmap&);

public Q_SLOTS:
    void setAvatar(const QPixmap& pixmap);
    void setAvatar(const QImage& image);
    void setText(const QString& text);
    void setComment(const QString& text);

private:
    QScopedPointer<class QtCardWidgetPrivate> d;
};

