#include "qtviewdrageventfilter.h"
#include <QScopedValueRollback>
#include <QPointer>
#include <QPoint>
#include <QModelIndex>
#include <QAbstractItemView>
#include <QScrollBar>
#include <QApplication>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <QDebug>

#include <QScroller>

#include <optional>

#include "../delegates/qtitemwidgetdelegate.h"
#include "../delegates/delegateinternals.h"

namespace
{
    class _ItemView : public QAbstractItemView
    {
    public:
        using QAbstractItemView::viewOptions;
    };
}


class QtViewDragEventFilterPrivate
{
public:
    QtViewDragEventFilter* q = nullptr;
    QPointer<QAbstractItemView> view;
    QtViewDragEventFilter::DragMoveMode dragMode = QtViewDragEventFilter::BoundedMove;
    QPixmap draggedPixmap;
    QModelIndex draggedIndex;
    bool paintLock = false;
    std::optional<QPoint> pressedPosition;
    std::optional<QPoint> draggedPosition;
    int yOffset = 0;
    int xOffset = 0;

    QtViewDragEventFilterPrivate(QtViewDragEventFilter* filter)
        : q(filter)
    {}

    void attachView(QAbstractItemView* v)
    {
        if (!v)
            return;

        v->installEventFilter(q);
        if (QScrollBar* sb = v->verticalScrollBar())
            sb->installEventFilter(q);
    }

    void detachView(QAbstractItemView* v)
    {
        if (!v)
            return;

        v->removeEventFilter(q);
        if (QScrollBar* sb = v->verticalScrollBar())
            sb->removeEventFilter(q);
    }

    QStyleOptionViewItem viewItemOption() const
    {
        return static_cast<_ItemView*>(view.data())->viewOptions();
    }

    QPixmap itemPixmap(const QModelIndex& index) const
    {
        if (!index.isValid())
            return {};

        QAbstractItemDelegate* delegate = view->itemDelegate();
        if (!delegate)
            return {};

        Qt5ExtraInternals::QtStyleOptionViewItemExtra option{ viewItemOption() };
        option.state |= QStyle::State_Selected;
        option.rect = view->visualRect(index);
        option.rect.moveTo(0, 0);
        option.renderDragItem = true;

        const qreal scale = view->devicePixelRatio();
        QPixmap pixmap(option.rect.size() * scale);
        pixmap.setDevicePixelRatio(scale);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        delegate->paint(&painter, option, index);
        painter.drawRect(pixmap.rect().adjusted(0, 0, -1, -1));

        return pixmap;
    }
};

QtViewDragEventFilter::QtViewDragEventFilter(QObject *parent, QAbstractItemView *v)
    : QGraphicsEffect(parent)
    , d(new QtViewDragEventFilterPrivate(this))
{
    setView(v);
}

QtViewDragEventFilter::~QtViewDragEventFilter() = default;

void QtViewDragEventFilter::setView(QAbstractItemView *view)
{
    d->detachView(d->view);
    d->view = view;
    d->attachView(d->view);
}

QAbstractItemView *QtViewDragEventFilter::view() const
{
    return d->view;
}

void QtViewDragEventFilter::setDragMoveMode(DragMoveMode m)
{
    d->dragMode = m;
}

QtViewDragEventFilter::DragMoveMode QtViewDragEventFilter::dragMoveMode() const
{
    return d->dragMode;
}

QModelIndex QtViewDragEventFilter::draggingIndex() const
{
    return d->draggedIndex;
}

void QtViewDragEventFilter::draw(QPainter* painter)
{
    if (Q_UNLIKELY(d->view == Q_NULLPTR) || !d->draggedPosition || d->draggedPixmap.isNull())
        return drawSource(painter);

    QPoint offset;
    QPixmap pixmap = sourcePixmap(Qt::DeviceCoordinates, &offset);
    QPoint position;

    QPainter* p = painter;
    QPainter pixmapPainter;
    if (!pixmap.isNull())
    {
        pixmapPainter.begin(&pixmap);
        p = &pixmapPainter;
    }

    switch (d->dragMode)
    {
    case UnboundedMove:
        position = *d->draggedPosition + QPoint{d->xOffset, d->yOffset };
        break;
    case BoundedMove:
    default:
        position = { 0, d->draggedPosition->y() + d->yOffset };
        break;
    }
    p->drawPixmap(position, d->draggedPixmap);

    if (!pixmap.isNull())
    {
        pixmapPainter.end();

        const QTransform restoreTransform = painter->worldTransform();
        painter->setWorldTransform({});
        painter->drawPixmap(offset, pixmap);
        painter->setWorldTransform(restoreTransform);
    }
}

bool QtViewDragEventFilter::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == d->view)
    {
        switch(event->type())
        {
        case QEvent::MouseButtonPress:
            return mousePressEvent(static_cast<QMouseEvent*>(event));
        case QEvent::MouseMove:
            return mouseMoveEvent(static_cast<QMouseEvent*>(event));
        case QEvent::MouseButtonRelease:
            return mouseReleaseEvent(static_cast<QMouseEvent*>(event));
        case QEvent::Leave:
        case QEvent::DragLeave:
            reset();
            break;
        default:
            break;
        }
    }
    if (d->view && watched == d->view->verticalScrollBar() && event->type() == QEvent::Wheel)
        return d->pressedPosition && d->draggedPosition;
    return QObject::eventFilter(watched, event);
}


bool QtViewDragEventFilter::mousePressEvent(QMouseEvent *e)
{
    if (Q_UNLIKELY(d->view == Q_NULLPTR))
        return false;

    if (d->view->selectionMode() != QAbstractItemView::SingleSelection)
    {
        qWarning() << "[QtViewDragEventFilter]: is not intended to be used in views with non-single selection mode";
        return false;
    }

    QPoint position = e->pos();
    const QRect itemRect = d->view->visualRect(d->view->indexAt(position));
    const QRect visibleRect = d->view->viewport()->visibleRegion().boundingRect();
    const QPoint p = position - visibleRect.topLeft() - itemRect.topLeft();

    QAbstractItemDelegate* delegate = d->view->itemDelegate();
    QStyleOptionViewItem option = d->viewItemOption();
    option.rect = itemRect;
    if (auto widgetDelegate = qobject_cast<QtItemWidgetDelegate*>(delegate))
    {
        if (!widgetDelegate->isOverDragArea(option, p))
            return false;
    }
    else if (!option.rect.contains(p))
    {
        return false;
    }

    d->view->setAutoScroll(false);
    d->pressedPosition = position;
    d->draggedPosition = position;

    position -= visibleRect.topLeft();
    d->yOffset = itemRect.y() - position.y();
    d->xOffset = itemRect.x() - position.x();

    return false;
}

bool QtViewDragEventFilter::mouseMoveEvent(QMouseEvent *e)
{
    if (Q_UNLIKELY(d->view == Q_NULLPTR))
    {
        reset();
        return false;
    }

    if (!d->pressedPosition || !d->draggedPosition)
        return false;

    d->draggedPosition = e->pos();
    const auto dragDist = (*d->draggedPosition - *d->pressedPosition).manhattanLength();
    if (dragDist > QApplication::startDragDistance() && !d->draggedIndex.isValid())
    {
        d->draggedIndex = d->view->indexAt(*d->draggedPosition);
        Q_EMIT dragIndexChanged(d->draggedIndex);
        d->draggedPixmap = d->itemPixmap(d->draggedIndex);
        d->view->doItemsLayout();
        d->view->viewport()->setCursor(Qt::DragMoveCursor);
    }
    else if (d->draggedIndex.isValid())
    {
        QAbstractItemModel* model = d->view->model();
        const QModelIndex current = d->view->indexAt(*d->draggedPosition);

        const QRect visibleRect = d->view->viewport()->rect();
        QRect itemRect = d->view->visualRect(d->draggedIndex);
        if (!visibleRect.contains(itemRect)) // TODO: improve smoothness
            d->view->scrollTo(d->draggedIndex);

        if (model && current.isValid() && current != d->draggedIndex)
        {
            // TODO: make it possible to use distances other than 1
            if (current.row() > d->draggedIndex.row())
                model->moveRow(d->draggedIndex.parent(), d->draggedIndex.row(), current.parent(), std::min(model->rowCount(), current.row() + 1));
            else
                model->moveRow(current.parent(), current.row(), d->draggedIndex.parent(), std::min(model->rowCount(), d->draggedIndex.row() + 1));


            const int k = current.row() > d->draggedIndex.row() ? 1 : -1;

            d->draggedIndex = current;
            Q_EMIT dragIndexChanged(d->draggedIndex);

            itemRect = d->view->visualRect(d->draggedIndex);
            if (!visibleRect.contains(itemRect))
                d->view->viewport()->scroll(0, 2 * k); // TODO: improve smoothness

            //d->view->scrollTo(d->draggedIndex);
            return true;
        }
        //d->view->doItemsLayout();
    }
    return false;
}

bool QtViewDragEventFilter::mouseReleaseEvent(QMouseEvent *e)
{
    reset();
    return false;
}

void QtViewDragEventFilter::reset()
{
    d->view->setAutoScroll(true);
    d->draggedIndex = {};
    Q_EMIT dragIndexChanged(d->draggedIndex);

    d->draggedPixmap = {};
    d->pressedPosition.reset();
    d->draggedPosition.reset();
    if (d->view)
    {
        d->view->doItemsLayout();
        d->view->viewport()->unsetCursor();
    }
}

