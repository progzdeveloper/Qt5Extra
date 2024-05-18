#include <QPainter>
#include <QEvent>
#include <QMouseEvent>
#include <QPixmapCache>
#include <QApplication>
#include <QScreen>
#include <QAbstractItemView>
#include <QPointer>
#include <QScopedValueRollback>

#include <QDebug>

#include <CacheMap> // from Qt5Extra aux

#include "qtitemwidgetdelegate.h"
#include "qtitemwidget.h"
#include "delegateinternals.h"

class QtItemWidgetDelegatePrivate
{
public:
    struct Hint
    {
        QSize size;
        QStyle::State state;

        bool operator==(const Hint& other) const noexcept
        {
            return size == other.size && state == other.state;
        }
        bool operator!=(const Hint& other) const noexcept
        {
            return !(*this == other);
        }
    };
    using PixmapCache = Qt5Extra::CacheMap<uint64_t, Hint, QPixmap>;

    static Q_CONSTEXPR size_t kDefaultPixmapCacheLimit = 64;
    static Q_CONSTEXPR size_t kDefaultPixmapCacheDepth = 4;

    QModelIndex currentIndex; // current model index (index that under mouse cursor)
    QModelIndex draggingIndex;
    mutable QScopedPointer<QtItemWidget> widget; // widget to embed
    mutable PixmapCache pixmapCache{ kDefaultPixmapCacheLimit, kDefaultPixmapCacheDepth };
    mutable int cachedWidth = 0; // cached item width - if it's changed we will drop the pixmap cache
    mutable double dpr = 1.0;
    QtItemWidgetDelegate::Options options = QtItemWidgetDelegate::NoOptions;
    QWidget::RenderFlags flags = QWidget::DrawChildren; // widget rendering flags

    static uint64_t packedIndex(const QModelIndex& index)
    {
        return uint64_t(index.row()) | (uint64_t(index.column()) << 32);
    }

    void renderDirect(QPainter* painter, const QRect& rect) const;
    void renderCached(QPainter* painter, const QStyleOptionViewItem &option, const QModelIndex &index, const QtItemWidgetDelegate *delegate) const;
    void updateDevicePixelRatio(const QScreen* screen);
};


void QtItemWidgetDelegatePrivate::renderDirect(QPainter *painter, const QRect &rect) const
{
    painter->save();
    painter->translate(rect.topLeft()); // translate painter
    widget->render(painter, QPoint(), QRegion(), flags); // render our embeded widget
    painter->restore();
}

void QtItemWidgetDelegatePrivate::renderCached(QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index, const QtItemWidgetDelegate *delegate) const
{
    // same as above - but use cache to speed-up things
    const uint64_t key = packedIndex(index);
    const Hint hint = { option.rect.size(), option.state };
    QPixmap pixmap;
    if (pixmapCache.find(key, hint, pixmap)) // cache hit
    {
        qDebug() << "cacheHit for (" << index.row() << index.column() << ')';
        painter->save();
        painter->translate(option.rect.topLeft());
        painter->drawPixmap(0, 0, pixmap); // draw pixmap from cache
        painter->restore();
    }
    else // cache miss
    {
        qDebug() << "cacheMiss for (" << index.row() << index.column() << ')';
        delegate->updateWidgetData(index, option); // update widget data
        pixmap = QPixmap(option.rect.size() * dpr);
        pixmap.setDevicePixelRatio(dpr);
        pixmap.fill(Qt::transparent);

        QPainter pixmapPainter(&pixmap);
        widget->render(&pixmapPainter, QPoint(), QRegion(), flags);

        pixmapCache.insert(key, hint, pixmap); // cache pixmap

        painter->drawPixmap(option.rect.topLeft(), pixmap);
    }
}

void QtItemWidgetDelegatePrivate::updateDevicePixelRatio(const QScreen* screen)
{
    if (!screen)
    {
        qWarning() << "failed to detect device pixel ratio: screen is nullptr";
        return;
    }

    const double currDpr = screen->devicePixelRatio();
    if (currDpr != dpr)
    {
        dpr = currDpr;
        pixmapCache.clear();
    }
}


QtItemWidgetDelegate::QtItemWidgetDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
    , d(new QtItemWidgetDelegatePrivate)
{
}

QtItemWidgetDelegate::~QtItemWidgetDelegate() = default;

void QtItemWidgetDelegate::setOptions(QtItemWidgetDelegate::Options options)
{
    if (d->options == options)
        return;

    d->options = options;
    d->flags.setFlag(QWidget::DrawWindowBackground, d->options & AutoFillBackground);
    if (d->options & StaticContents)
        d->options.setFlag(CacheItemPixmap, true);

    if (!(d->options & CacheItemPixmap))
        d->pixmapCache.clear();
}

QtItemWidgetDelegate::Options QtItemWidgetDelegate::options() const
{
    return d->options;
}

void QtItemWidgetDelegate::setCacheLimit(int cacheSize)
{
    d->pixmapCache.resize(std::max(0, cacheSize));
}

int QtItemWidgetDelegate::cacheLimit() const
{
    return d->pixmapCache.capacity();
}

bool QtItemWidgetDelegate::isOverDragArea(const QStyleOptionViewItem& option, const QPoint& p) const
{
    if (Q_UNLIKELY(d->widget == Q_NULLPTR))
        return false;

    d->widget->setGeometry(option.rect);
    return d->widget->dragArea().contains(p);
}

void QtItemWidgetDelegate::setDragIndex(const QModelIndex &index)
{
    d->draggingIndex = index;
}

QModelIndex QtItemWidgetDelegate::dragIndex() const
{
    return d->draggingIndex;
}

void QtItemWidgetDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!index.isValid())
        return;

    const auto* extraOptions = qstyleoption_cast<const Qt5ExtraInternals::QtStyleOptionViewItemExtra*>(&option);
    if (d->draggingIndex == index && extraOptions && !extraOptions->renderDragItem)
    {
        painter->fillRect(option.rect, option.palette.base());
        return;
    }

    createWidgetOnDemand(); // lazy widget creation
    if (Q_UNLIKELY(d->widget == Q_NULLPTR))
        return QStyledItemDelegate::paint(painter, option, index);

    if (index == d->currentIndex)
        d->widget->applyState();
    else
        d->widget->clearState();

    if (option.widget)
    {
        const int width = option.widget->width();
        if (width != d->cachedWidth)
            d->pixmapCache.clear();
        d->cachedWidth = width;
    }

    if (auto view = qobject_cast<const QAbstractItemView*>(option.widget))
    {
        // Workaround for Qt bug (it doesn't set QStyle::State_MouseOver in some circumstances)
        // This situation can occur when some rows/columns was inserted/removed from model
        // but we have not receive any mouse events, so QStyle::State_MouseOver was not updated.
        // Here we determine it using QCursor::pos() and reset option.state accordingly
        if (auto viewport = view->viewport())
        {
            const QPoint localPos = viewport->mapFromGlobal(QCursor::pos());
            if (option.rect.contains(localPos))
                const_cast<QStyleOptionViewItem&>(option).state |= QStyle::State_MouseOver;
            else
                const_cast<QStyleOptionViewItem&>(option).state &= ~QStyle::State_MouseOver;
        }
    }

    // draw item backdground
    if ((d->options & HighlightSelected) && (option.state & QStyle::State_Selected))
        painter->fillRect(option.rect, option.palette.highlight());
    else if ((d->options & HighlightHovered) && (option.state & QStyle::State_MouseOver))
        painter->fillRect(option.rect, option.palette.color(QPalette::Highlight).lighter(180));

    // evaluate how to draw widget - does we need caching
    // or update widget data and draw directly
    const RenderHint hint = renderHint(option, index);
    if (hint == RenderDirect)
    {
        updateWidgetData(index, option);
        // resize widget if needed
        if (d->widget->size() != option.rect.size())
            d->widget->resize(option.rect.size());

        d->renderDirect(painter, option.rect);
    }
    else
    {
        d->renderCached(painter, option, index, this);
    }
}

// Hint for QtWidgetItemDelegate::paint() method - does we need
// to render from cache or bypass cache and render directly
// Derived classes can overload this method to provide custom behavior
QtItemWidgetDelegate::RenderHint QtItemWidgetDelegate::renderHint(const QStyleOptionViewItem &option, const QModelIndex &) const
{
    // if state mouse over or selected we always draw widget directly (to avoid artefacts)
    // if caching was disabled explicitly - always draw widget directly
    // in other cases - we try to use cache
    const bool isSelectedOrHovered = (option.state & (QStyle::State_MouseOver | QStyle::State_Selected));
    const bool isCachingEnabled = d->options & CacheItemPixmap;
    const bool isStaticContents = d->options & StaticContents;
    if ((isCachingEnabled || isStaticContents) && !isSelectedOrHovered)
        return RenderCached;
    else
        return RenderDirect;
}

QtItemWidget *QtItemWidgetDelegate::widget() const
{
    return d->widget.get();
}

QSize QtItemWidgetDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!index.isValid())
        return QStyledItemDelegate::sizeHint(option, index);

    createWidgetOnDemand();
    if (Q_UNLIKELY(d->widget == Q_NULLPTR))
        return QStyledItemDelegate::sizeHint(option, index);

    // use widget minimum size as sizeHint for item
    return d->widget->minimumSize();
}

void QtItemWidgetDelegate::createWidgetOnDemand() const
{
    if (d->widget)
        return;

    d->widget.reset(createItemWidget());
    if (Q_UNLIKELY(d->widget == Q_NULLPTR))
        return;

    // This is a most important thing - set an
    // Qt::WA_DontShowOnScreen attribute on widget
    // to disable widget ability to self-render
    d->widget->setAttribute(Qt::WA_DontShowOnScreen);
    d->widget->setMouseTracking(true);
    // This also important: we adjust widget size beforehand
    // to get correct value for sizeHint() overriden method
    d->widget->adjustSize();
}

void QtItemWidgetDelegate::updateWidgetData(const QModelIndex& index, const QStyleOptionViewItem& option) const
{
    if (d->widget)
        d->widget->setData(index, option); // setup widget from data in index
}

bool QtItemWidgetDelegate::eventHandler(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    // this is a single point that implements event handling:
    // no matter from where we got an event - it can be a
    // editorEvent() overriden or eventFilter() overriden method, but not both
    if (!d->widget || !model || !index.isValid())
    {
        if (d->options & CustomEventFilter)
            return false;
        else
            return QStyledItemDelegate::editorEvent(event, model, option, index);
    }

    QWidget* widget = const_cast<QWidget*>(option.widget);
    QWidget* viewport = nullptr;
    QAbstractItemView* view = qobject_cast<QAbstractItemView*>(widget);
    if (view)
        viewport = view->viewport();

    if (!view || !viewport)
        return QStyledItemDelegate::editorEvent(event, model, option, index);

    if (model->rowCount() == 0 || model->columnCount() == 0)
        view->unsetCursor(); // no rows or no columns, model is empty - reset cursor


    /*switch (event->type())
    {
    case QEvent::KeyPress: qDebug() << "QtItemWidgetDelegate KeyPress Event"; break;
    case QEvent::KeyRelease: qDebug() << "QtItemWidgetDelegate KeyRelease Event"; break;
    case QEvent::MouseMove: qDebug() << "QtItemWidgetDelegate MouseMove Event"; break;
    case QEvent::MouseButtonPress: qDebug() << "QtItemWidgetDelegate MouseButtonPress Event"; break;
    case QEvent::MouseButtonRelease: qDebug() << "QtItemWidgetDelegate MouseButtonRelease Event"; break;
    default:
        break;
    }*/

    switch (event->type())
    {
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
    case QEvent::MouseMove:
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    {
        if (d->currentIndex != index) // index mismatch - update cuurrent index
            d->widget->setData(index, option);

        d->currentIndex = index;
        invalidateIndex(d->currentIndex);
        d->widget->applyState();

        // move widget (this save us from coping event data)
        d->widget->move(viewport->mapToGlobal(option.rect.topLeft()));
        d->widget->setData(d->currentIndex, option);
        if (d->options & StaticContents)
            d->widget->viewportEvent(event, widget, option); // resend event directly
        else
            d->widget->handleEvent(event, widget, option); // resend event directly, while updating current state

        // force to repaint item rect
        view->doItemsLayout();

        return QStyledItemDelegate::editorEvent(event, model, option, index);
    }
    default:
        break;
    }

    if (d->options & CustomEventFilter)
        return false;
    else
        return QStyledItemDelegate::editorEvent(event, model, option, index);
}

bool QtItemWidgetDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (d->options & CustomEventFilter)
        return QStyledItemDelegate::editorEvent(event, model, option, index);
    else
        return eventHandler(event, model, option, index);
}

bool QtItemWidgetDelegate::eventFilter(QObject *object, QEvent *event)
{
    if (d->options & CustomEventFilter)
        return QObject::eventFilter(object, event);

    QWidget* viewport = Q_NULLPTR;
    QAbstractItemView* view = qobject_cast<QAbstractItemView*>(parent());
    if (view)
        viewport = view->viewport();

    if (!view || !viewport)
        return QObject::eventFilter(object, event);

    QAbstractItemModel* model = view->model();
    if (!model)
        return QObject::eventFilter(object, event);

    if (object == viewport || object == view)
    {
        QModelIndex index = view->indexAt(viewport->mapFromGlobal(QCursor::pos()));
        QStyleOptionViewItem optionView;
        optionView.initFrom(viewport);
        optionView.rect = view->visualRect(index);
        optionView.decorationSize = view->iconSize();
        optionView.widget = view;
        return eventHandler(event, model, optionView, index);
    }
    return QObject::eventFilter(object, event);
}

void QtItemWidgetDelegate::invalidate()
{
    d->pixmapCache.clear();
    Q_EMIT requestRepaint();
}

void QtItemWidgetDelegate::invalidateIndex(const QModelIndex &index)
{
    d->pixmapCache.erase(d->packedIndex(index));
}

void QtItemWidgetDelegate::invalidateRange(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    const QModelIndex index = topLeft;
    for (int i = topLeft.row(), n = bottomRight.row(); i <= n; ++i)
        for (int j = topLeft.column(), m = bottomRight.column(); j <= m; ++j)
            d->pixmapCache.erase(d->packedIndex(index));
}
