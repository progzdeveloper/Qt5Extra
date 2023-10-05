#pragma once
#include <QtWidgetsExtra>
#include <QFrame>

class QLabel;
class QtBadgeEffect;
class QtTextLabel;

class QtCardWidget :
    public QFrame
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY(QString comment READ comment WRITE setComment NOTIFY commentChanged)
    Q_PROPERTY(Qt::ToolButtonStyle cardStyle READ cardStyle WRITE setCardStyle NOTIFY cardStyleChanged)
    Q_PROPERTY(int avatarRoundness READ avatarRoundness WRITE setAvatarRoundness NOTIFY avatarRoundnessChanged)
public:
    explicit QtCardWidget(QWidget *parent = Q_NULLPTR);
    ~QtCardWidget();

    void setBadgeValue(const QVariant& value);
    QVariant badgeValue() const;

    QtBadgeEffect &badge();

    void setCardStyle(Qt::ToolButtonStyle style);
    Qt::ToolButtonStyle cardStyle() const;

    // factor < 0 -> Circle
    // factor > 0 -> Rounded
    // factor = 0 -> Rect
    void setAvatarRoundness(int factor);
    int avatarRoundness() const;

    QtTextLabel* textLabel() const;
    QtTextLabel* commentLabel() const;
    QLabel* avatarLabel() const;

    QPixmap avatar() const;
    QString text() const;
    QString comment() const;

Q_SIGNALS:
    void textChanged(const QString&);
    void commentChanged(const QString&);
    void avatarChanged(const QPixmap&);
    void cardStyleChanged(Qt::ToolButtonStyle);
    void avatarRoundnessChanged(int);

public Q_SLOTS:
    void setAvatar(const QPixmap& pixmap);
    void setAvatar(const QImage& image);
    void setText(const QString& text);
    void setComment(const QString& text);

private:
    QScopedPointer<class QtCardWidgetPrivate> d;
};

