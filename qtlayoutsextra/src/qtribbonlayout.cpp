#include "qtribbonlayout.h"
#include "qtlayoututils.h"
#include "layoutinternals.h"
#include <cmath>
#include <QtGeometryAlgorithms>


class QtRibbonLayoutPrivate :
        public Qt5ExtraInternals::LayoutAssistant
{
public:
    typedef QSize(QLayoutItem::* LayoutItemSize)() const;

    QRect geometry_;
    QList<QLayoutItem*> items_;
    QLayoutItem* leaderItem_ = nullptr;
    Qt::Orientation orientation_ = Qt::Horizontal;

    QtRibbonLayoutPrivate(QtRibbonLayout* layout, Qt::Orientation orientation)
        : LayoutAssistant(layout)
        , orientation_(orientation)
    {}

    QSize advanceSize(const QSize& total, const QSize& s) const
    {
        if (orientation_ == Qt::Horizontal)
            return { total.width() + s.width(), std::max(s.height(), total.height()) };
        else
            return { std::max(total.width(), s.width()), total.height() + s.height() };
    }

    QSize itemsSize(int spacing, LayoutItemSize itemSize) const
    {
        QSize result;
        for (const auto* item : items_)
        {
            if (leaderItem_ && item == leaderItem_)
                continue;

            result = advanceSize(result, (item->*itemSize)());
        }

        if (!items_.empty())
        {
            const int sp = spacing * (items_.size() - 1);
            if (orientation_ == Qt::Horizontal)
                result.rwidth() += sp;
            else
                result.rheight() += sp;
        }
        return result;
    }

    QSize findSize(const QtRibbonLayout* layout, LayoutItemSize itemSize, LayoutItemSize leaderSize) const
    {
        QSize size = itemsSize(std::max(0, layout->spacing()), itemSize);
        if (leaderItem_)
            size = advanceSize(size, (leaderItem_->*leaderSize)());

        size += marginsSize(layout->contentsMargins());
        return size.boundedTo({ QLAYOUTSIZE_MAX, QLAYOUTSIZE_MAX });
    }

    void doLayout(const QtRibbonLayout* layout, const QRect& rect)
    {
        if (items_.empty())
            return;

        const int sp = std::max(0, layout->spacing());
        QRect rc = rect;
        if (!leaderItem_)
        {
            if (orientation_ == Qt::Horizontal)
                layoutItemsHorizontally(rc, 0, items_.size(), sp);
            else
                layoutItemsVertically(rc, 0, items_.size(), sp);
            return;
        }

        const int leaderIndex = layout->indexOf(leaderItem_);
        Q_ASSERT(leaderIndex != -1);
        if (leaderIndex == -1)
            return;

        const QSize size = itemsSize(sp, &QLayoutItem::sizeHint);
        if (orientation_ == Qt::Horizontal)
        {
            const int w = size.width() / 2;
            leaderItem_->setGeometry(rect.adjusted(w, 0, -w, 0));
            rc = leaderItem_->geometry();

            const QRect leftRect = { rect.left(), rect.top(), rc.left() - rect.left() - sp, rect.height() };
            layoutItemsHorizontally(leftRect, 0, leaderIndex, sp);

            const QRect rightRect = { rc.right() + sp, rect.top(), rect.right() - rc.right() - sp, rect.height() };
            layoutItemsHorizontally(rightRect, leaderIndex + 1, items_.size(), sp);
        }
        else
        {
            const int h = size.height() / 2;
            leaderItem_->setGeometry(rect.adjusted(0, h, 0, -h));
            rc = leaderItem_->geometry();

            const QRect topRect = { rect.left(), rect.top(), rect.width(), rc.top() - rect.top() - sp };
            layoutItemsVertically(topRect, 0, leaderIndex, sp);

            const QRect bottomRect = { rect.left(), rc.bottom() + sp, rect.width(), rect.bottom() - rc.bottom() - sp };
            layoutItemsVertically(bottomRect, leaderIndex + 1, items_.size(), sp);
        }
    }

    void layoutItemsHorizontally(const QRect& rect, int first, int last, int spacing)
    {
        if (first > last)
            return;

        const int n = (last - first);
        if (n == 0)
            return;

        const int iw = (rect.width() - (n - 1) * spacing) / n;
        const int ih = rect.height();

        int x = rect.x(), y = rect.y();
        for (; first < last; ++first, x += (iw + spacing))
        {
            const QRect itemRect{ x, y, iw, ih };

            QLayoutItem* item = items_[first];
            QRect rc{ {}, item->sizeHint() };
            rc = adjustedRect(rc, itemRect, { item->alignment(), Qt::IgnoreAspectRatio, RectFitPolicy::CropSource });
            item->setGeometry(rc);
        }
    }

    void layoutItemsVertically(const QRect& rect, int first, int last, int spacing)
    {
        if (first > last)
            return;

        const int n = (last - first);
        if (n == 0)
            return;

        const int iw = rect.width();
        const int ih = (rect.height() - (n - 1) * spacing) / n;

        int x = rect.x(), y = rect.y();
        for (; first < last; ++first, y += ih + spacing)
        {
            const QRect itemRect{ x, y, iw, ih };

            QLayoutItem* item = items_[first];
            QRect rc{ {}, item->sizeHint() };
            rc = adjustedRect(rc, itemRect, { item->alignment(), Qt::IgnoreAspectRatio, RectFitPolicy::CropSource });
            item->setGeometry(rc);
        }
    }
};


QtRibbonLayout::QtRibbonLayout(Qt::Orientation orientation)
    : QtRibbonLayout(nullptr, orientation)
{
}

QtRibbonLayout::QtRibbonLayout(QWidget* parent, Qt::Orientation orientation)
    : QLayout(parent)
    , d(new QtRibbonLayoutPrivate(this, orientation))
{
}

QtRibbonLayout::~QtRibbonLayout()
{
    qDeleteAll(d->items_);
}

void QtRibbonLayout::setOrientation(Qt::Orientation orientation)
{
    if (d->orientation_ == orientation)
        return;

    d->orientation_ = orientation;
    invalidate();

    Q_EMIT orientationChanged(d->orientation_);
}

Qt::Orientation QtRibbonLayout::orientation() const
{
    return d->orientation_;
}

bool QtRibbonLayout::setLeaderItem(QLayoutItem* item)
{
    if (!item)
        return false;

    if (d->items_.count(item) > 0 && d->leaderItem_ != item)
    {
        d->leaderItem_ = item;
        invalidate();
        return true;
    }
    return false;
}

QLayoutItem* QtRibbonLayout::leaderItem() const
{
    return d->leaderItem_;
}

Qt::Orientations QtRibbonLayout::expandingDirections() const
{
    return d->orientation_;
}

QSize QtRibbonLayout::sizeHint() const
{
    return d->findSize(this, &QLayoutItem::minimumSize, &QLayoutItem::minimumSize);
}

QSize QtRibbonLayout::minimumSize() const
{
    return d->findSize(this, &QLayoutItem::sizeHint, &QLayoutItem::minimumSize);
}

QSize QtRibbonLayout::maximumSize() const
{
    return d->findSize(this, &QLayoutItem::maximumSize, &QLayoutItem::maximumSize);
}

void QtRibbonLayout::setGeometry(const QRect& geometry)
{
    invalidate();
    d->geometry_ = geometry;
    QLayout::setGeometry(geometry);
    d->doLayout(this, geometry.marginsRemoved(contentsMargins()));
}

void QtRibbonLayout::addWidget(QWidget* widget, Qt::Alignment alignment)
{
    if (!d->checkWidget(widget))
        return;

    QLayout::addWidget(widget); // calls addItem() under the hood
    if (auto item = itemAt(count() - 1))
        item->setAlignment(alignment);
}

void QtRibbonLayout::insertWidget(int index, QWidget* widget, Qt::Alignment alignment)
{
    if (!d->checkWidget(widget))
        return;

    addChildWidget(widget);
    if (index < 0) // append
        index = d->items_.count();

    QWidgetItem* item = d->createWidgetItem(widget, alignment);
    d->items_.insert(index, item);
    invalidate();
}

void QtRibbonLayout::addItem(QLayoutItem* item)
{
    if (!d->checkItem(item))
        return;

    if (QLayout* layout = item->layout())
        adoptLayout(layout);

    d->items_.append(item);
    invalidate();
}

QLayoutItem* QtRibbonLayout::itemAt(int index) const
{
    return d->items_.value(index, nullptr);
}

QLayoutItem* QtRibbonLayout::takeAt(int index)
{
    auto result = d->items_.takeAt(index);
    if (result)
    {
        if (result == d->leaderItem_)
            d->leaderItem_ = nullptr;
        invalidate();
    }
    return result;
}

int QtRibbonLayout::count() const
{
    return d->items_.count();
}
