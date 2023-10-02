#pragma once
#include <QtWidgetsExtra>
#include <QAbstractButton>

class QTWIDGETSEXTRA_EXPORT QtSlideButton :
        public QAbstractButton
{
    Q_OBJECT
public:
    explicit QtSlideButton(QWidget *parent = Q_NULLPTR);
    ~QtSlideButton();

    QSize sizeHint() const Q_DECL_OVERRIDE;

protected:
    bool hitButton(const QPoint& p) const Q_DECL_OVERRIDE;
    void paintEvent(QPaintEvent *e) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *e) Q_DECL_OVERRIDE;

private:
    QScopedPointer<class QtSlideButtonPrivate> d;
};

