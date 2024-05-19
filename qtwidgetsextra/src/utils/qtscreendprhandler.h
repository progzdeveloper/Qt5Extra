#pragma once
#include <QObject>
#include <QtWidgetsExtra>

class QScreen;
class QTWIDGETSEXTRA_EXPORT QtScreenDprHandler : public QObject
{
    Q_OBJECT
public:
    explicit QtScreenDprHandler(QWidget* w);
    ~QtScreenDprHandler();

    void setWidget(QWidget* w);
    QWidget* widget() const;

Q_SIGNALS:
    void screenDprChanged(double dpr);
    void screenRectChanged(const QRect&);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void onScreenChanged(QScreen* screen);

private:
    friend class QtScreenDprHandlerPrivate;
    QScopedPointer<class QtScreenDprHandlerPrivate> d;
};
