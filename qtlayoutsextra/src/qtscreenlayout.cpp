﻿#include "qtscreenlayout.h"
#include <QLayoutItem>
#include <QApplication>
#include <QScreen>
#include <QPointer>
#include <QWidget>
#include <QWindow>
#include <QSet>
#include <QScopedValueRollback>

#include <QtGeometryAlgorithms>
#include <QtRectLayouts>

#include "qtgridpagelayout_p.h"

#include <deque>

namespace
{
    class ScreenItem : public QLayoutItem
    {
    public:
        explicit ScreenItem(QWidget* widget = Q_NULLPTR) : w(widget) {}
        // QLayoutItem interface
    public:
        QSize sizeHint() const Q_DECL_OVERRIDE { return w ? w->sizeHint() : QSize{}; }
        QSize minimumSize() const Q_DECL_OVERRIDE { return w ? w->minimumSize() : QSize{}; }
        QSize maximumSize() const Q_DECL_OVERRIDE { return w ? w->maximumSize() : QSize{}; }
        Qt::Orientations expandingDirections() const Q_DECL_OVERRIDE { return Qt::Horizontal | Qt::Vertical;}
        void setGeometry(const QRect& r) Q_DECL_OVERRIDE
        {
            if (w)
            {
                w->resize(r.size());
                w->move(r.topLeft());
            }
        }
        QRect geometry() const Q_DECL_OVERRIDE { return w ? w->geometry() : QRect{}; }
        bool isEmpty() const Q_DECL_OVERRIDE { return w == nullptr;}
        QWidget *widget() Q_DECL_OVERRIDE { return w; }
    private:
        QPointer<QWidget> w;
    };
}


class QtScreenLayoutPrivate
{
public:
    using RectCache = QVarLengthArray<QRect, 4>;

    QSet<QScreen*> usedScreens;
    QtScreenLayout* q = Q_NULLPTR;
    QPointer<QScreen> screen;
    std::deque<QLayoutItem*> items;
    QRect customRect;
    Qt::Orientation orientation = Qt::Horizontal;
    mutable Qt::Alignment pivotAlign = Qt::Alignment{0};
    QtScreenLayout::ScreenMode screenMode = QtScreenLayout::FullGeometry;
    QtScreenLayout::LayoutMode layoutMode = QtScreenLayout::BoxMode;
    QtScreenLayout::EnqueueMode enqueueMode = QtScreenLayout::EnqueueBack;
    int maxScreens = -1;
    bool locked = false;

    QtScreenLayoutPrivate(QtScreenLayout* layout, QScreen* scr)
        : q(layout)
        , screen(scr)
    {
    }

    ~QtScreenLayoutPrivate()
    {
        qDeleteAll(items);
    }

    void layoutItemsInBox(const QRect& screenRect)
    {
        if (items.empty())
            return;

        QScopedValueRollback guard(locked, true);
        QtBoxRectLayout::Options options;
        options.align = Qt::AlignCenter;
        options.orientation = orientation;
        options.spacing = q->spacing();

        RectCache rects;
        itemsRects(rects);
        const QRect bounds = alignedRect(QtBoxRectLayout::boundingRect(options, rects.begin(), rects.end()), screenRect, q->alignment());
        QtBoxRectLayout::layoutRects(bounds, options, rects.begin(), rects.end(), rects.begin());
        setItemsRects(rects);
    }

    void layoutItemsInGrid(const QRect& screenRect)
    {
        if (items.empty())
            return;

        RectCache rects;
        itemsRects(rects);

        QSize maxItemSize;
        for (const auto& r : rects)
            maxItemSize = maxItemSize.expandedTo(r.size());

        Qt5ExtraInternals::GridSize gridSize;
        QtGridRectLayout::minGridSize(screenRect.size(), maxItemSize, static_cast<int>(items.size()), gridSize.rows, gridSize.cols);

        const QRect bounds = alignedRect({ QPoint{}, QtGridRectLayout::boundingSize(gridSize.rows, gridSize.cols, maxItemSize) }, screenRect, q->alignment());

        rects.resize(gridSize.count());
        QtGridRectLayout::layoutRects(bounds, gridSize.rows, gridSize.cols, gridSize.count(), rects.begin());
        setItemsRects(rects);
    }

    void layoutItemsInCascade(const QRect& screenRect)
    {
        if (items.empty())
            return;

        QScopedValueRollback guard(locked, true);
        RectCache rects;
        itemsRects(rects);
        QRect bounds = { screenRect.topLeft(), QSize{} };

        const QPoint delta = { q->spacing(), q->spacing() };
        QPoint offset;
        for (QRect& r : rects)
        {
            r.moveTopLeft(offset);
            offset += delta;
            bounds |= r;
        }

        bounds = alignedRect(bounds, screenRect, q->alignment());
        offset = bounds.topLeft();
        for (QRect& r : rects)
        {
            r.moveTopLeft(offset);
            offset += delta;
        }

        setItemsRects(rects);
    }


    void layoutItems(const QRect& screenRect)
    {
        switch(layoutMode)
        {
        case QtScreenLayout::CascadeMode:
            layoutItemsInCascade(screenRect);
            break;
        case QtScreenLayout::GridMode:
            layoutItemsInGrid(screenRect);
            break;
        case QtScreenLayout::BoxMode:
        default:
            layoutItemsInBox(screenRect);
            break;
        }
    }

    QRect pivotRect(const QRect& r) const
    {
        if (pivotAlign == Qt::Alignment{0})
            pivotAlign = q->alignment();
        return alignedRect(r, effectiveRect(), pivotAlign);
    }

    void itemsRects(RectCache& cache)
    {
        cache.clear();
        for (auto* item : items)
            cache << QRect{ QPoint{}, item->minimumSize() };
    }

    void setItemsRects(const RectCache& cache)
    {
        if (cache.empty())
            return;

        const bool isAnimated = q->isAnimated() && q->isAnimationAllowed();
        const QRect pivot = pivotRect(cache.front());

        QPoint offset;
        const int n = std::min(static_cast<int>(items.size()), cache.size());
        for (int i = 0; i < n; ++i)
        {
            QLayoutItem* item = items[i];
            QWidget* w = item->widget();
            QRect r = items[i]->geometry();
            if (w && !w->isVisible())
            {
                r = pivot;
                item->setGeometry(r);
            }

            if (isAnimated)
                q->animateItem(q, item, r, cache[i]);
            else
                item->setGeometry(cache[i]);
        }
    }

    int indexOf(QWidget* w) const
    {
        for (int i = 0, n = static_cast<int>(items.size()); i < n; ++i)
            if (w == items[i]->widget())
                return i;
        return -1;
    }

    QRect effectiveRect() const
    {
        QRect rect;
        switch(screenMode)
        {
        case QtScreenLayout::CustomGeometry:
            rect = customRect.isValid() ? customRect : screen->geometry();
            break;
        case QtScreenLayout::AvailGeometry:
            rect = screen->availableGeometry();
            break;
        case QtScreenLayout::FullGeometry:
        default:
            rect = screen->geometry();
            break;
        }
        return rect.marginsRemoved(q->contentsMargins());
    }

    QSize screenSize() const
    {
        switch(screenMode)
        {
        case QtScreenLayout::AvailGeometry:
            return screen->availableSize();
        case QtScreenLayout::FullGeometry:
        default:
            break;
        }
        return screen->size();
    }

    QScreen* nextAvailScreen() const
    {
        const auto screens = qApp->screens();
        for (QScreen* scr : screens)
        {
            if (!usedScreens.contains(scr))
                return scr;
        }
        return qApp->primaryScreen();
    }

    void attachScreen(QScreen* scr)
    {
        if (!scr)
            return;

        switch(screenMode)
        {
        case QtScreenLayout::AvailGeometry:
            QObject::connect(scr, &QScreen::availableGeometryChanged, q, &QLayout::setGeometry);
            break;
        case QtScreenLayout::FullGeometry:
            QObject::connect(scr, &QScreen::geometryChanged, q, &QLayout::setGeometry);
            break;
        default:
            break;
        }
    }

    void detachScreen(QScreen* scr)
    {
        if (!scr)
            return;

        switch(screenMode)
        {
        case QtScreenLayout::AvailGeometry:
            QObject::connect(scr, &QScreen::availableGeometryChanged, q, &QLayout::setGeometry);
            break;
        case QtScreenLayout::FullGeometry:
            QObject::connect(scr, &QScreen::geometryChanged, q, &QLayout::setGeometry);
            break;
        default:
            break;
        }
    }

    static QScreen* resolveScreen(QScreen* scr)
    {
        if (scr)
            return scr;

        QWidget* activeWidget = qApp->activeWindow();
        QWindow* window = activeWidget ? activeWidget->windowHandle() : Q_NULLPTR;
        scr = window ? window->screen() : Q_NULLPTR;
        return scr ? scr : QApplication::primaryScreen();
    }
};


QtScreenLayout::QtScreenLayout(QScreen *scr)
    : d(new QtScreenLayoutPrivate(this, QtScreenLayoutPrivate::resolveScreen(scr)))
{
    Q_ASSERT(d->screen != Q_NULLPTR); // screen pointer can't be nullptr
    d->attachScreen(d->screen);
    QtScreenLayout::setGeometry(d->effectiveRect());

    connect(qApp, &QApplication::screenAdded, this, &QtScreenLayout::onScreenAdded);
    connect(qApp, &QApplication::screenRemoved, this, &QtScreenLayout::onScreenRemoved);
}

QtScreenLayout::~QtScreenLayout() = default;

void QtScreenLayout::setScreen(QScreen *scr)
{
    if (d->screen == scr)
        return;

    d->detachScreen(d->screen);
    d->screen = scr;
    d->attachScreen(d->screen);
    setGeometry(d->effectiveRect());
}

QScreen *QtScreenLayout::screen() const
{
    return d->screen;
}

void QtScreenLayout::onScreenAdded(QScreen*)
{
    // reserved for future implementations
}

void QtScreenLayout::onScreenRemoved(QScreen *scr)
{
    if (scr != d->screen)
        return;

    invalidate();
    d->detachScreen(d->screen);
    d->screen = QtScreenLayoutPrivate::resolveScreen(Q_NULLPTR);
    d->attachScreen(d->screen);
    // don't play any animation
    QScopedValueRollback(d->locked, true);
    setAnimated(false);
    d->layoutItems(d->effectiveRect());
    setAnimated(true);
}

void QtScreenLayout::setMaxUseableScreens(int n)
{
    if (n == d->maxScreens)
        return;

    const bool requireLayout = n != -1 && n < d->maxScreens;
    d->maxScreens = n;
    if (requireLayout)
    {
        invalidate();
        d->layoutItems(d->effectiveRect());
    }
}

int QtScreenLayout::maxUseableScreens() const
{
    return d->maxScreens;
}

void QtScreenLayout::setLayoutMode(LayoutMode mode)
{
    if (d->layoutMode == mode)
        return;

    d->layoutMode = mode;
    invalidate();
    d->layoutItems(d->effectiveRect());
}

QtScreenLayout::LayoutMode QtScreenLayout::layoutMode() const
{
    return d->layoutMode;
}

void QtScreenLayout::setEnqueueMode(EnqueueMode mode)
{
    if (d->enqueueMode == mode)
        return;

    d->enqueueMode = mode;
    invalidate();
    d->layoutItems(d->effectiveRect());
}

QtScreenLayout::EnqueueMode QtScreenLayout::enqueueMode() const
{
    return d->enqueueMode;
}

void QtScreenLayout::setScreenMode(ScreenMode mode)
{
    if (d->screenMode == mode)
        return;

    d->screenMode = mode;
    invalidate();
    d->layoutItems(d->effectiveRect());
}

QtScreenLayout::ScreenMode QtScreenLayout::screenMode() const
{
    return d->screenMode;
}

void QtScreenLayout::setOrientation(Qt::Orientation orientation)
{
    if (d->orientation == orientation)
        return;

    d->orientation = orientation;
    if (d->layoutMode == BoxMode)
    {
        invalidate();
        d->layoutItems(d->effectiveRect());
    }
}

Qt::Orientation QtScreenLayout::orientation() const
{
    return d->orientation;
}

void QtScreenLayout::setPivotAlignment(Qt::Alignment align)
{
    d->pivotAlign = align;
}

Qt::Alignment QtScreenLayout::pivotAlignment() const
{
    return d->pivotAlign;
}

QSize QtScreenLayout::sizeHint() const
{
    return d->screenSize();
}

QSize QtScreenLayout::minimumSize() const
{
    return d->screenSize();
}

QSize QtScreenLayout::maximumSize() const
{
    return d->screenSize();
}

Qt::Orientations QtScreenLayout::expandingDirections() const
{
    return Qt::Horizontal | Qt::Vertical;
}

void QtScreenLayout::updateGeometry(const QRect &r)
{
    QScopedValueRollback(d->locked, true);
    d->customRect = r;
    setAnimated(false);
    d->layoutItems(d->effectiveRect());
    setAnimated(true);
}

void QtScreenLayout::setGeometry(const QRect& r)
{
    d->layoutItems(r.marginsRemoved(contentsMargins()));
}

QRect QtScreenLayout::geometry() const
{
    return d->effectiveRect();
}

QLayoutItem *QtScreenLayout::appendWidget(QWidget *widget)
{
    QWidget* w = widget->window();
    if (!w || d->indexOf(w) != -1)
        return nullptr;

    w->adjustSize();
    w->installEventFilter(this);
    connect(w, &QObject::destroyed, this, [this](QObject* w) { takeAt(indexOf(static_cast<QWidget*>(w))); });
    QLayoutItem* item = new ScreenItem(w);
    addItem(item);
    return item;
}

void QtScreenLayout::addItem(QLayoutItem *item)
{
    invalidate();
    switch (d->enqueueMode)
    {
    case EnqueueFront:
        d->items.emplace_front(item);
        break;
    case EnqueueBack:
        d->items.emplace_back(item);
        break;
    default:
        Q_ASSERT("unreachable");
    }

    d->layoutItems(d->effectiveRect());
}

QLayoutItem *QtScreenLayout::itemAt(int index) const
{
    if (index < 0 || index >= static_cast<int>(d->items.size()))
        return Q_NULLPTR;

    return d->items[index];
}

QLayoutItem *QtScreenLayout::takeAt(int index)
{
    if (index < 0 || index >= static_cast<int>(d->items.size()))
        return Q_NULLPTR;

    auto it = d->items.begin() + index;
    QLayoutItem* item = *it;
    d->items.erase(it);
    if (QWidget* w = item->widget())
        w->removeEventFilter(this);
    d->layoutItems(d->effectiveRect());
    return item;
}

int QtScreenLayout::count() const
{
    return static_cast<int>(d->items.size());
}

bool QtScreenLayout::eventFilter(QObject *watched, QEvent *event)
{
    if (d->locked || !watched->isWidgetType())
        return QtAnimatedLayout::eventFilter(watched, event);

    const int index = d->indexOf(static_cast<QWidget*>(watched));
    if (index < 0 || index >= static_cast<int>(d->items.size()))
        return QtAnimatedLayout::eventFilter(watched, event);

    switch(event->type())
    {
    case QEvent::Close:
        takeAt(index); // remove if widget was closed
    default:
        break;
    }
    return QtAnimatedLayout::eventFilter(watched, event);
}
