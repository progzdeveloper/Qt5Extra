#include "aspectratiolayout.h"
#include "gridpagelayout.h"
#include "layoutinternals.h"

#include <QGridLayout>
#include <QWidget>

#include <QtGeometryAlgorithms>

#include <memory>
#include <cmath>


class AspectRatioLayoutPrivate
        : public Qt5ExtraInternals::LayoutAssistant
{
public:
    static constexpr double kMinAspectRatio = 0.0001; // to avoid zero division

    double aspectRatio_ = 1.0;
    std::unique_ptr<QLayoutItem> item_; // at most one

    AspectRatioLayoutPrivate(QLayout* layout, double ratio)
        : LayoutAssistant(layout)
        , aspectRatio_(std::max(ratio, kMinAspectRatio))
    {}

    int scaled(int width) const
    {
        return (width * aspectRatio_);
    }

    void adjustToGrid(const QRect& r, const QGridLayout* layout, int& h, int& w) const
    {
        if (layout->count() == 1)
            return;

        /* The issue is that QGridLayout::rowCount() doesn't actually return
             * the number of rows that you can see, it actually returns the number
             * of rows that QGridLayout has internally allocated for rows of data
             * (yes, this isn't very obvious and isn't documented).
             * The reason is that QGridLayout can only grow not shrink.
             * To get around this you can either delete the QGridLayout and recreate
             * it, or if you're convinced that your column count won't change (and
             * this is our case), you can divide total number of items inside layout
             * on number of columns to get actual number of rows.
             */
        const int rows = std::ceil(layout->count() / static_cast<double>(layout->columnCount()));
        const int cols = layout->columnCount();
        if (rows == 0 || cols == 0)
            return; // avoid division by zero

        if (rows < cols)
            h = scaled(w) / cols;
        else if (rows > cols)
            h = scaled(w / cols) * rows;

        if (h > r.height())
        {
            h = r.height();
            w = h / aspectRatio_ * cols / rows;
        }
    }

    void adjustToGrid(const QRect& r, const GridPageLayout* layout, int& h, int& w) const
    {
        if (layout->count() == 1)
            return;

        const int rows = layout->rowCount();
        const int cols = layout->columnCount();
        if (rows == 0 || cols == 0)
            return; // avoid division by zero

        h = scaled(w / cols) * rows;
        if (h > r.height())
        {
            h = r.height();
            w = h / aspectRatio_ * cols / rows;
        }
    }

    static bool isMinMaxContraint(QLayout::SizeConstraint constraint)
    {
        return constraint == QLayout::SetMinAndMaxSize;
    }
};

AspectRatioLayout::AspectRatioLayout(QWidget* parent, double ratio)
    : AnimatedLayout(parent)
    , d(new AspectRatioLayoutPrivate(this, ratio))
{
}

AspectRatioLayout::~AspectRatioLayout() = default;

void AspectRatioLayout::setAspectRatio(double value)
{
    value = std::max(value, d->kMinAspectRatio);
    if (qFuzzyCompare(value, d->aspectRatio_))
        return;

    d->aspectRatio_ = value;

    Q_EMIT aspectRatioChanged(value);
    invalidate();
    activate();
}

double AspectRatioLayout::aspectRatio() const
{
    return d->aspectRatio_;
}

int AspectRatioLayout::count() const
{
    return static_cast<int>(d->item_ != nullptr);
}

QLayoutItem* AspectRatioLayout::itemAt(int index) const
{
    return index == 0 ? d->item_.get() : nullptr;
}

QLayoutItem* AspectRatioLayout::takeAt(int)
{
    return d->item_.release();
}

Qt::Orientations AspectRatioLayout::expandingDirections() const
{
    return  Qt::Horizontal | Qt::Vertical; // we'd like grow beyond sizeHint() in both directions
}

bool AspectRatioLayout::hasHeightForWidth() const
{
    return true;
}

int AspectRatioLayout::heightForWidth(int width) const
{
    if (!d->isMinMaxContraint(sizeConstraint()) || !geometry().isValid())
        return d->scaled(width);

    const int h = d->scaled(width);
    if (!d->item_)
        return h;

    return std::min(h, d->item_->minimumSize().height());
}

QRect AspectRatioLayout::effectiveRect(const QRect& rect, QLayout* hint) const
{
    const QRect r = rect.marginsRemoved(contentsMargins());
    int w = r.width();
    int h = d->isMinMaxContraint(sizeConstraint()) ? d->scaled(w) : heightForWidth(w);
    if (hint)
    {
        if (QGridLayout* grid = qobject_cast<QGridLayout*>(hint))
            d->adjustToGrid(r, grid, h, w);
        else if (GridPageLayout* customGrid = qobject_cast<GridPageLayout*>(hint))
            d->adjustToGrid(r, customGrid, h, w);
    }
    if (h > r.height())
    {
        h = r.height();
        w = h / d->aspectRatio_;
    }

    QRect rc{ 0, 0, w, h };
    rc.moveCenter(r.center());
    return rc;
}

void AspectRatioLayout::setGeometry(const QRect& rect)
{
    if (!d->item_)
        return;

    const QRect rc = effectiveRect(rect, d->item_->layout());
    QLayout::setGeometry(rc);
    d->item_->setGeometry(rc);
}

QSize AspectRatioLayout::sizeHint() const
{
    const QSize size = marginsSize(contentsMargins());
    return d->item_ ? d->item_->sizeHint() + size : size;
}

QSize AspectRatioLayout::minimumSize() const
{
    const QSize size = marginsSize(contentsMargins());
    return d->item_ ? d->item_->minimumSize() + size : size;
}

QSize AspectRatioLayout::maximumSize() const
{
    return (d->isMinMaxContraint(sizeConstraint()) && d->item_) ? d->item_->maximumSize() : QLayout::maximumSize();
}

void AspectRatioLayout::setLayout(QLayout* layout, Qt::Alignment alignment)
{
    setItem(layout, alignment);
}

void AspectRatioLayout::setWidget(QWidget* widget, Qt::Alignment alignment)
{
    if (!d->checkWidget(widget))
        return;

    addChildWidget(widget);
    setItem(d->createWidgetItem(widget), alignment);
}

void AspectRatioLayout::setItem(QLayoutItem* item, Qt::Alignment alignment)
{
    if (!d->checkItem(item))
        return;

    if (auto layout = item->layout())
        adoptLayout(layout);

    const QRect rc = geometry();
    invalidate();
    addItem(item);
    d->item_->setAlignment(alignment);
    setGeometry(rc);
    activate();
}

void AspectRatioLayout::addItem(QLayoutItem* item)
{
    d->item_.reset(item);
    if (item)
        item->setAlignment({});
}
