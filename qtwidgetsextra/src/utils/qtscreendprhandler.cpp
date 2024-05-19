#include "qtscreendprhandler.h"
#include <QPointer>
#include <QScreen>
#include <QWidget>
#include <QWindow>
#include <QDebug>

class QtScreenDprHandlerPrivate
{
public:
    QtScreenDprHandler* q = nullptr;
    QPointer<QWidget> widget;
    QPointer<QScreen> currentScreen;
    double currentDpr = 1.0;

    QtScreenDprHandlerPrivate(QtScreenDprHandler* handler)
        : q(handler)
    {}

    void attach()
    {
        if (!widget)
            return;

        QWidget* wnd = widget->window();
        if (Q_UNLIKELY(wnd == nullptr))
            return;

        wnd->winId(); // force creating native handle to have ability to obtain windowHandle()
        QWindow* winHandle = wnd->windowHandle();
        Q_ASSERT(winHandle != Q_NULLPTR);

        QObject::connect(winHandle, &QWindow::screenChanged, q, &QtScreenDprHandler::onScreenChanged);

        currentScreen = winHandle->screen();
        Q_ASSERT(currentScreen != Q_NULLPTR);
        QObject::connect(currentScreen, &QScreen::geometryChanged, q, &QtScreenDprHandler::screenRectChanged);

        widget->installEventFilter(q);
    }

    void detach()
    {
        if (currentScreen)
            QObject::disconnect(currentScreen, &QScreen::geometryChanged, q, &QtScreenDprHandler::screenRectChanged);

        if (!widget)
            return;

        widget->removeEventFilter(q);
        QWidget* wnd = widget->window();
        if (Q_UNLIKELY(wnd == nullptr))
            return;

        QObject::disconnect(wnd->windowHandle(), &QWindow::screenChanged, q, &QtScreenDprHandler::onScreenChanged);
    }

    void updateDpr(double dpr)
    {
        if (currentDpr != dpr)
        {
            qDebug() << "[" << widget << "]: device pixel ratio changed" << currentDpr << "->" << dpr;
            currentDpr = dpr;
            Q_EMIT q->screenDprChanged(currentDpr);
        }
    }
};


QtScreenDprHandler::QtScreenDprHandler(QWidget* w)
    : QObject(w)
    , d(new QtScreenDprHandlerPrivate(this))
{
    d->widget = w;
    d->attach();
}

QtScreenDprHandler::~QtScreenDprHandler() = default;

void QtScreenDprHandler::setWidget(QWidget* w)
{
    d->detach();
    d->widget = w;
    setParent(d->widget);
    d->attach();
}

QWidget* QtScreenDprHandler::widget() const
{
    return d->widget;
}

void QtScreenDprHandler::onScreenChanged(QScreen* screen)
{
    if (!screen)
        return;

    qDebug() << "[" << d->widget << "]: screen was changed";
    disconnect(d->currentScreen, &QScreen::geometryChanged, this, &QtScreenDprHandler::screenRectChanged);

    d->currentScreen = screen;
    connect(d->currentScreen, &QScreen::geometryChanged, this, &QtScreenDprHandler::screenRectChanged);
    Q_EMIT screenRectChanged(d->currentScreen->geometry());

    d->updateDpr(d->currentScreen->devicePixelRatio());
}

bool QtScreenDprHandler::eventFilter(QObject* watched, QEvent* event)
{
    if (d->widget && watched == d->widget && event->type() == QEvent::Paint)
        d->updateDpr(d->widget->devicePixelRatioF());

    return QObject::eventFilter(watched, event);
}

