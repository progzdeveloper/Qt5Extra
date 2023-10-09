#include <unordered_map>

#include <QPainter>
#include <QEvent>
#include <QMouseEvent>
#include <QPixmapCache>
#include <QApplication>
#include <QAbstractItemView>
#include <QPointer>
#include <QScopedValueRollback>

#include <QDebug>


#include "qtitemwidgetdelegate.h"
#include "qtitemwidget.h"

namespace
{
Q_CONSTEXPR int kDefaultPixmapCacheLimit = 64;

// Compute hash of model index and size of it cell representation.
// Hashing is doing by first packing row/column of index
// and width/height of size into 64-bit unsigned integers
// and reinterpreting them as a QLatin-1 string: this is
// one of the fastest methods
QByteArray uniqueName(const QModelIndex& index, const QSize& size)
{
#ifdef QT_DEBUG
    Q_CONSTEXPR size_t kMaxBufSize = 512;
    Q_CONSTEXPR const char* fmt = "%i:%i_%ix%i#%i";
    QByteArray readable;
    const int n = qsnprintf(nullptr, 0, fmt, index.row(), index.column(), size.width(), size.height());
    readable.resize(n);
    qsnprintf(readable.data(), kMaxBufSize, fmt, index.row(), index.column(), size.width(), size.height());
    return readable;
#else
    char buf[2 * sizeof(uint64_t)];

    uint64_t h1 = uint64_t(index.row()) | uint64_t(index.column()) << 32; // pack index
    memcpy(buf + sizeof(uint64_t) * 0, &h1, sizeof(uint64_t)); // a la std::bit_cast

    uint64_t h2 = uint64_t(size.width()) | uint64_t(size.height()) << 32; // pack size
    memcpy(buf + sizeof(uint64_t) * 1, &h2, sizeof(uint64_t)); // a la std::bit_cast

   return QByteArray{ buf, sizeof(buf) }; // reinterpret as a byte array
#endif
}

}


class PixmapCache
{
    struct Hasher
    {
        inline size_t operator()(const QByteArray& ba) const Q_DECL_NOTHROW
        {
            return qHash(ba);
        }
    };

public:
    explicit PixmapCache(int limit)
        : maxSize(limit)
    {
        storage.reserve(limit);
    }

    void setCacheLimit(int limit)
    {
        maxSize = static_cast<int>(std::max(0, limit));
    }

    int cacheLimit() const
    {
        return static_cast<int>(maxSize);
    }

    void clear()
    {
        storage.clear();
    }

    bool find(const QByteArray& key, QPixmap& result) const
    {
        auto it = storage.find(key);
        if (it != storage.end())
        {
            result = it->second;
            return true;
        }
        return false;
    }

    void insert(const QByteArray& key, const QPixmap& pixmap)
    {
        auto result = storage.emplace(key, pixmap);
        if (result.second)
            shrink(result.first);
        else
            result.first->second = pixmap;
    }

    void remove(const QByteArray& key)
    {
        storage.erase(key);
    }

private:
    template<class _It>
    void shrink(_It pos)
    {
        if (maxSize < 1)
            return;

        auto it = storage.begin();
        while(storage.size() > maxSize)
        {
            if (it != pos)
                it = storage.erase(it);
            else
                ++it;
        }
    }

private:
    std::unordered_map<QByteArray, QPixmap, Hasher> storage;
    size_t maxSize;
};


class QtItemWidgetDelegatePrivate
{
public:
    mutable QModelIndex currentIndex; // current model index (index that under mouse cursor)
    mutable QPointer<QtItemWidget> widget; // widget to embed
    mutable PixmapCache pixmapCache{ kDefaultPixmapCacheLimit };
    mutable int cachedWidth = 0; // cacitem width - if it's changed we will drop the pixmap cache
    QtItemWidgetDelegate::Options options = QtItemWidgetDelegate::NoOptions;
    QWidget::RenderFlags flags = QWidget::DrawChildren; // widget rendering flags

    void renderDirect(QPainter* painter, const QRect& rect) const;
    void renderCached(QPainter* painter, const QStyleOptionViewItem &option, const QModelIndex &index, const QtItemWidgetDelegate *delegate) const;
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
    const QByteArray id = uniqueName(index, option.rect.size());

    QPixmap pixmap;
    if (pixmapCache.find(id, pixmap)) // cache hit
    {
        painter->save();
        painter->translate(option.rect.topLeft());
        painter->drawPixmap(0, 0, pixmap); // draw pixmap from cache
        painter->restore();
    }
    else // cache miss
    {
        delegate->updateWidgetData(index, option); // update widget data
        pixmap = QPixmap(option.rect.size());
        pixmap.fill(Qt::transparent);

        QPainter pixmapPainter(&pixmap);
        widget->render(&pixmapPainter, QPoint(), QRegion(), flags);

        pixmapCache.insert(id, pixmap); // cache pixmap

        painter->drawPixmap(option.rect.topLeft(), pixmap);
    }
}


QtItemWidgetDelegate::QtItemWidgetDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
    , d(new QtItemWidgetDelegatePrivate)
{
}

void QtItemWidgetDelegate::setOptions(QtItemWidgetDelegate::Options options)
{
    if (d->options == options)
        return;

    d->options = options;
    d->flags.setFlag(QWidget::DrawWindowBackground, d->options & AutoFillBackground);
    if (!(d->options & CacheItemPixmap))
        d->pixmapCache.clear();
}

QtItemWidgetDelegate::Options QtItemWidgetDelegate::options() const
{
    return d->options;
}

QtItemWidgetDelegate::~QtItemWidgetDelegate() = default;

void QtItemWidgetDelegate::setCacheLimit(int cacheSize)
{
    d->pixmapCache.setCacheLimit(cacheSize);
}

int QtItemWidgetDelegate::cacheLimit() const
{
    return d->pixmapCache.cacheLimit();
}

void QtItemWidgetDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!index.isValid())
        return;

    createWidgetOnDemand(); // lazy widget creation
    if ((d->widget == Q_NULLPTR))
        return QStyledItemDelegate::paint(painter, option, index);

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

    // resize widget if needed
    if (d->widget->size() != option.rect.size())
        d->widget->resize(option.rect.size());

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
    if (isCachingEnabled && isStaticContents)
        return RenderCached;
    if (isCachingEnabled && !isSelectedOrHovered)
        return RenderCached;
    return RenderDirect;
}

QtItemWidget *QtItemWidgetDelegate::widget() const
{

    return d->widget;
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

    d->widget = createItemWidget();
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
    {
        d->widget->setData(index, option); // setup widget from data in index
    }
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

    switch (event->type())
    {
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
    case QEvent::MouseMove:
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    {
        // move widget (this save us from coping event data)
        d->widget->move(viewport->mapToGlobal(option.rect.topLeft()));

        if (d->currentIndex != index) // index mismatch - update cuurrent index
        {
            d->widget->setData(index, option);
        }

        d->currentIndex = index;

        if (d->options & StaticContents)
            d->widget->viewportEvent(event, widget, option); // resend event directly
        else
            d->widget->handleEvent(event, widget, option); // resend event directly, while updating current state

        // force to repaint item rect
        viewport->repaint();

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
    if (auto view = qobject_cast<QAbstractItemView*>(parent()))
        d->pixmapCache.remove(uniqueName(index, view->visualRect(index).size()));
}

void QtItemWidgetDelegate::invalidateRange(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    auto view = qobject_cast<QAbstractItemView*>(parent());
    if (!view)
        return;

    const QModelIndex index = topLeft;
    for (int i = topLeft.row(), n = bottomRight.row(); i <= n; ++i)
        for (int j = topLeft.column(), m = bottomRight.column(); j <= m; ++j)
            d->pixmapCache.remove(uniqueName(index.sibling(i, j), view->visualRect(index).size()));
}
