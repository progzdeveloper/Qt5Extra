#include "gridpagelayout.h"
#include "gridpagelayout_p.h"
#include "layoututils.h"
#include "layoutinternals.h"
#include <QtGeometryAlgorithms>
#include <QtObjectWatcher>

#include <deque>
#include <unordered_set>
#include <iterator>
#include <algorithm>
#include <cmath>

#include <QAbstractAnimation>
#include <QWidget>

#include <QVarLengthArray>

class GridPageLayoutPrivate
        : public LayoutInternals::LayoutAssistant
        , public LayoutInternals::GridOptions
{
public:
    using AnimationSet = std::unordered_set<QAbstractAnimation*>;
    using ItemsMap = std::deque<QLayoutItem*>;
    using ItemRange = LayoutInternals::IterSpan<ItemsMap>;

    typedef QSize(QLayoutItem::* LayoutItemSize)() const;

    ItemRange makeRange(int page, int itemsPerPage) const noexcept
    {
        const size_t pageIndex = static_cast<size_t>(page);
        const size_t itemIndex = (pageIndex * itemsPerPage);
        const size_t itemsSize = items.size();
        if (page < 0 || itemIndex >= itemsSize)
            return { items, itemsSize, itemsSize };

        if (itemsSize < static_cast<size_t>(itemsPerPage)) // just a single page
            return { items, 0, itemsSize };

        size_t first = pageIndex * itemsPerPage;
        size_t last = first + itemsPerPage;
        switch (fillMode)
        {
        case FillMode::AllowUnfilled:
            return { items, first, std::min(last, itemsSize) };
        case FillMode::RetainFilled:
            first -= last > itemsSize ? (last - itemsSize) : 0;
            return { items, first, std::min(last, itemsSize) };
        default:
            break;
        }
        return { items, itemsSize, itemsSize };
    }

    template<class _Fn>
    static void scanExcluded(ItemsMap& items, const ItemRange& range, _Fn func)
    {
        std::for_each(items.begin(), items.begin() + range.lower(), func);
        std::for_each(items.begin() + range.upper(), items.end(), func);
    }


    GridPageLayoutPrivate(GridPageLayout* layout)
        : LayoutAssistant(layout)
        , q(layout)
    {}

    ~GridPageLayoutPrivate()
    {
        qDeleteAll(items);
    }

    int itemsPerPage() const { return currSize.count(); }

    QSize boundingSize(const LayoutInternals::GridSize& size) const
    {
        return boundingSize(size.rows, size.cols);
    }

    QSize boundingSize(int rows, int cols) const
    {
        const int sp = q->spacing();
        const QSize ms = marginsSize(q->contentsMargins());
        const int w = minCellSize.width() * cols + sp * cols +  ms.width();
        const int h = minCellSize.height() * rows + sp * rows + ms.height();
        return { w, h };
    }

    QSize adjustByAspectRatio(const QSize& size, int rows, int cols) const
    {
        if (aspectRatio != 0.0 && rows != 0 && cols != 0) // avoid zero-division
        {
            int w = size.width();
            int h = q->heightForWidth(w / cols) * rows;
            if (h > size.height())
            {
                h = size.height();
                w = h / aspectRatio * cols / rows;
            }
            return { w, h };
        }
        return size;
    }

    QRect effectiveRect(const QRect& rect, int rows, int cols) const
    {
        const QRect contentsRect = rect.marginsRemoved(q->contentsMargins());
        QRect rc = contentsRect;
        if (q->sizeConstraint() == QLayout::SetMinimumSize)
        {
            const QSize size = boundingSize(rows, cols);
            rc.setSize(size.boundedTo(adjustByAspectRatio(size, rows, cols)));
        }
        else
        {
            rc.setSize(adjustByAspectRatio(rc.size(), rows, cols));
        }
        return adjustedRect(rc, contentsRect, { q->alignment(), Qt::IgnoreAspectRatio, RectFitPolicy::CropSource });
    }

    QSize findSize(int page, int desiredCols, int nrows, int ncols, LayoutItemSize size) const
    {
        const ItemRange range = makeRange(page, itemsPerPage());
        size_t n = range.size();

        const QMargins margins = q->contentsMargins();
        if (n == 0 || ncols == 0 || nrows == 0)
            return marginsSize(margins);

        const size_t offset = range.lower();
        const int sp = q->spacing();
        const auto remained = nrows * ncols - n;

        int w = 0; // result width
        int h = 0; // result height

        int cw = 0; // current row accumulated width
        int ch = 0; // current column accumulated height

        // traverse cols (most long)
        for (size_t i = 0; i < n; ++i)
        {
            const QSize itemSize = (items[i + offset]->*size)();
            cw += itemSize.width();
            if (!((i + 1) % ncols))
            {
                w = std::max(w, cw);
                cw = 0;
            }
        }

        // traverse rows (most tall)
        n = ncols - remained;
        for (size_t c = 0; c < n; ++c)
        {
            for (int r = 0; r < nrows; ++r)
            {
                const int idx = r * desiredCols + c + offset;
                const QSize itemSize = (items[idx]->*size)();
                ch += itemSize.height();
            }
            h = std::max(ch, h);
            ch = 0;
        }

        // add spacing and margins
        w += sp * (ncols - 1) + horizontalMargins(margins);
        h += sp * (nrows - 1) + verticalMargins(margins);
        // clamp to layout maximum size
        return QSize{ std::min(w, QLAYOUTSIZE_MAX), std::min(h, QLAYOUTSIZE_MAX) };
    }

    void doLayout(const QRect& rect, int page, bool suppressAnimation = false)
    {
        const ItemRange range = makeRange(page, itemsPerPage());

        scanExcluded(items, range, [this](auto item) { setItemRect(item, {}); });

        int n = static_cast<int>(range.size());
        const int offset = range.lower();
        const int nrows = q->rowCount();
        const int ncols = q->columnCount();
        const auto remained = nrows * ncols - n;
        const int sp = q->spacing();

        if (nrows == 0 || ncols == 0) // wrap around
        {
            const bool hasPrevPage = q->hasPrev();
            const bool hasNextPage = q->hasNext();
            if (hasPrevPage && !hasNextPage)
                q->prevPage();
            else if (hasNextPage && !hasPrevPage)
                q->nextPage();
            return;
        }

        const int spw = sp * (ncols - 1);
        const int sph = sp * (nrows - 1);

        // single item width and height
        const int iw = std::max((rect.width() - spw) / ncols, minCellSize.width());
        const int ih = std::max((rect.height() - sph) / nrows, minCellSize.height());

        QRect itemRect{ 0, 0, iw, ih };
        int x = rect.left();
        int y = rect.top();
        int dx = iw + sp;
        int dy = ih + sp;

        const bool hasRemaining = (flowAlign == Qt::AlignCenter || flowAlign == Qt::AlignJustify) && (remained != 0);
        n = nrows - hasRemaining;
        for (int r = 0; r < n; ++r, y += dy)
        {
            x = rect.left();
            for (int c = 0; c < ncols; ++c, x += dx)
            {
                const int i = r * currSize.cols + c + offset;
                itemRect.moveTo(x, y);
                layoutItem(itemRect, i, suppressAnimation);
            }
        }

        if (!hasRemaining)
            return;

        const int icount = currSize.cols - remained;
        const QRect rowRect{ rect.x(), y, rect.width(), rect.height() };
        QRect rowItemsRect = rowRect;
        if (q->alignment() == Qt::AlignJustify)
        {
            // distribute the width between remained items, i.e. stretch items width
            itemRect.setWidth(rect.width() / icount);
            dx = itemRect.width() + sp;
        }
        else
        {
            // minimal bounding rect of remained items, i.e. keep items aspect ratio
            rowItemsRect = { 0, 0, iw * icount + sp * (icount - 1), ih };
            rowItemsRect.moveCenter(rowRect.center());
        }

        x = rowItemsRect.left();
        for (int c = 0; c < icount; ++c, x += dx)
        {
            const int i = (nrows - 1) * currSize.cols + c + offset;
            itemRect.moveTo(x, y);
            layoutItem(itemRect, i, suppressAnimation);
        }
    }

    void layoutItem(const QRect& rect, QLayoutItem* item, bool suppressAnimation = false)
    {
        if (!item)
            return;

        // the following code is rely on that QSizePolicy::retainSizeWhenHidden()
        // feature of widget is enabled/overrided
        QRect rc{ QPoint{}, clampedSize(rect.size(), item->minimumSize(), item->maximumSize()) };
        rc = adjustedRect(rc, rect);

        if (suppressAnimation || !(q->isAnimated() && q->isAnimationAllowed()))
        {
            setItemRect(item, rc);
            return;
        }

        QWidget* w = q->parentWidget();
        if (!w || !w->isVisible())
        {
            // if widget isn't visible we supress animation and force item geometry setup
            // to avoid that item has incorrect geometry afterwhile widget become visible
            setItemRect(item, rc);
            return;
        }

        if (QAbstractAnimation* animation = q->createAnimation(q, item, item->geometry(), rc))
            animation->start(QAbstractAnimation::DeleteWhenStopped);
    }

    void layoutItem(const QRect& rect, int i, bool suppressAnimation = false)
    {
        layoutItem(rect, q->itemAt(i), suppressAnimation);
    }

    void setItemRect(QLayoutItem* item, const QRect& r)
    {
        if (!item)
            return;

        item->setGeometry(r); // setup item geometry
        if (QWidget* w = item->widget())
            w->setVisible(r.isValid()); // update widget visibility
    }

    void overrideRetainSizePolicy(QWidget* w)
    {
        // The grid layout required to set retain size, when widget is hidden to true, since otherwise
        // the layout routines can't distinguish between hidden and visible widgets. That leads to
        // incorrect geometry calculations of the layout. Here we force that feature to be true.
        QSizePolicy policy = w->sizePolicy();
        policy.setRetainSizeWhenHidden(true);
        w->setSizePolicy(policy);
    }

    void emplaceAnimation(QAbstractAnimation* animation, const char* propertyName, QLayoutItem* item)
    {
        animation->setProperty(propertyName, QVariant::fromValue((void*)item));
        QObject::connect(animation, &QAbstractAnimation::destroyed, q, [this](QObject* object) {
            animations.erase(static_cast<QAbstractAnimation*>(object));
        });

        animations.emplace(animation);
    }

    void removeAnimation(const QLayoutItem* item, const char* propertyName)
    {
        auto it = std::find_if(animations.begin(), animations.end(),
                               [item, propertyName](const QObject* a) { return a->property(propertyName).value<void*>() == item; });
        if (it != animations.end())
        {
            (*it)->stop();
            animations.erase(it);
        }
    }

    GridPageLayout* q = nullptr;
    GridPageLayout::AdjustOptions adjustOptions;
    ItemsMap items;
    AnimationSet animations;
    QSize cachedMinSize, cachedMaxSize, cachedSizeHint;
    AnimationFeatures animationFeatures = AnimationFeature::AnimateItemsReorder;
    int currentPage = 0;
};


GridPageLayout::GridPageLayout(QWidget* widget)
    : AnimatedLayout(widget)
    , d(new GridPageLayoutPrivate(this))
{
    setAlignment(Qt::AlignCenter);
    setAnimated(d->animationFeatures != NoAnimation);
}

GridPageLayout::~GridPageLayout() = default;

void GridPageLayout::setAspectRatio(double ratio)
{
    ratio = std::max(0.0, ratio);
    if (d->aspectRatio == ratio)
        return;

    d->aspectRatio = ratio;
    Q_EMIT aspectRatioChanged(d->aspectRatio);
    invalidate();
}

double GridPageLayout::aspectRatio() const
{
    return d->aspectRatio;
}

void GridPageLayout::setFlowAlignment(Qt::Alignment alignment)
{
    if (d->flowAlign == alignment)
        return;

    d->flowAlign = alignment;
    Q_EMIT flowAlignmentChanged(alignment);
    invalidate();
}

Qt::Alignment GridPageLayout::flowAlignment() const
{
    return d->flowAlign;
}

void GridPageLayout::setGridFlow(GridPageLayout::GridFlow type)
{
    if (d->flowKind == type)
        return;

    d->flowKind = type;
    if (d->flowKind == StaticBoundedGrid)
        setFixedGridSize(d->maxSize.rows, d->maxSize.cols);

    Q_EMIT gridFlowChanged(type);
    invalidate();
}

GridPageLayout::GridFlow GridPageLayout::gridFlow() const
{
    return d->flowKind;
}

void GridPageLayout::setFillMode(GridPageLayout::PageFillMode mode)
{
    if (d->fillMode == mode)
        return;

    d->fillMode = mode;
    Q_EMIT fillModeChanged(mode);
    invalidate();
}

GridPageLayout::PageFillMode GridPageLayout::fillMode() const
{
    return d->fillMode;
}

void GridPageLayout::setAnimationFeatures(GridPageLayout::AnimationFeatures features)
{
    if (d->animationFeatures == features)
        return;

    d->animationFeatures = features;
    setAnimated(features != NoAnimation);
}

GridPageLayout::AnimationFeatures GridPageLayout::animationFeatures() const
{
    return d->animationFeatures;
}

void GridPageLayout::setAdjustOptions(const GridPageLayout::AdjustOptions& options)
{
    d->adjustOptions = options;
    d->adjustOptions.minAdjustingItemCount = std::max(0, options.minAdjustingItemCount);
    d->adjustOptions.itemCountTolerance = std::clamp(options.itemCountTolerance, 0, d->adjustOptions.minAdjustingItemCount);
}

const GridPageLayout::AdjustOptions& GridPageLayout::adjustOptions() const
{
    return d->adjustOptions;
}

int GridPageLayout::rowCount(int page) const
{
    page = page < 0 ? d->currentPage : page;
    const auto range = d->makeRange(page, d->itemsPerPage());
    const int n = static_cast<int>(range.size());
    const int r = static_cast<int>(std::ceil(n / static_cast<double>(d->currSize.cols)));
    return (n == 0 ? 0 : std::clamp(n, 1, r));
}

int GridPageLayout::columnCount(int page) const
{
    page = page < 0 ? d->currentPage : page;
    const auto range = d->makeRange(page, d->itemsPerPage());
    const int n = static_cast<int>(range.size());
    const int m = n < d->minSize.count() ? 1 : d->minSize.cols;
    return (n == 0 ? 0 : std::clamp(n, m, d->currSize.cols));
}

void GridPageLayout::setMaxColumnCount(int n)
{
    n = std::max(1, n);
    if (n == d->maxSize.cols)
        return;

    d->maxSize.cols = n;
    d->currSize.cols = n;
    Q_EMIT maxColumnCountChanged(n);

    if (d->flowKind == StaticBoundedGrid)
        setMinColumnCount(n);

    invalidate();
}

int GridPageLayout::maxColumnCount() const
{
    return d->maxSize.cols;
}

void GridPageLayout::setMinColumnCount(int n)
{
    n = std::max(1, n);
    if (n == d->minSize.cols)
        return;

    d->minSize.cols = n;
    d->currSize.cols = n;
    Q_EMIT minColumnCountChanged(n);

    if (d->flowKind == StaticBoundedGrid)
        setMaxColumnCount(n);

    invalidate();
}

int GridPageLayout::minColumnCount() const
{
    return d->minSize.cols;
}

void GridPageLayout::setMaxRowCount(int n)
{
    n = std::max(1, n);
    if (n == d->maxSize.rows)
        return;

    d->maxSize.rows = n;
    d->currSize.rows = n;
    Q_EMIT maxRowCountChanged(n);

    if (d->flowKind == StaticBoundedGrid)
        setMinRowCount(n);

    invalidate();
}

int GridPageLayout::maxRowCount() const
{
    return d->maxSize.rows;
}

void GridPageLayout::setMinRowCount(int n)
{
    n = std::max(1, n);
    if (n == d->minSize.rows)
        return;

    d->minSize.rows = n;
    d->currSize.rows = n;
    Q_EMIT maxRowCountChanged(n);

    if (d->flowKind == StaticBoundedGrid)
        setMaxRowCount(n);

    invalidate();
}

int GridPageLayout::minRowCount() const
{
    return d->minSize.rows;
}

bool GridPageLayout::isFixedSize() const
{
    return d->isFixedSize();
}

void GridPageLayout::setMinimumCellSize(const QSize& s)
{
    if (d->minCellSize == s)
        return;

    d->minCellSize = s;
    Q_EMIT minimumCellSizeChanged(s);
    invalidate();
}

QSize GridPageLayout::minimumCellSize() const
{
    return d->minCellSize;
}

int GridPageLayout::itemIndex(int row, int column, int page) const
{
    if (page < 0)
        page = d->currentPage;

    const auto range = d->makeRange(page, d->itemsPerPage());
    const size_t index = row * d->maxSize.cols + column + range.lower();
    if (index > d->items.size())
        return -1;
    return static_cast<int>(index);
}

int GridPageLayout::itemPage(int index) const
{
    return index / d->itemsPerPage();
}

int GridPageLayout::itemsPerPage() const
{
    return d->itemsPerPage();
}

int GridPageLayout::count() const
{
    return static_cast<int>(d->items.size());
}

int GridPageLayout::addWidget(QWidget* widget)
{
    if (!d->checkWidget(widget))
        return -1;

    QLayout::addWidget(widget); // calls addItem() under the hood
    return static_cast<int>(d->items.size());
}

void GridPageLayout::addItem(QLayoutItem* item)
{
    if (!d->checkItem(item))
        return;

    if (QWidget* w = item->widget()) // override hidden retain size policy on widget
        d->overrideRetainSizePolicy(w); // see details here

    const int npages = pageCount();
    d->items.push_back(item);
    invalidate();
    const int n = pageCount();
    if (npages != n)
        Q_EMIT pageCountChanged(n);
}

QLayoutItem* GridPageLayout::itemAt(int i) const
{
    if (i < 0 || i >= static_cast<int>(d->items.size()))
        return nullptr;

    return d->items[i];
}

QLayoutItem* GridPageLayout::takeAt(int i)
{
    if (i < 0 || i >= static_cast<int>(d->items.size()))
        return Q_NULLPTR;

    auto it = d->items.begin() + i;

    QLayoutItem* retval = *it;
    d->items.erase(it);
    invalidate();

    const int npages = pageCount();
    const int ipp = itemsPerPage();
    const int idx = d->currentPage * ipp;
    const int page = idx / ipp;
    const auto range = d->makeRange(page, ipp);
    d->scanExcluded(d->items, range, [this](auto item) { d->setItemRect(item, {}); });
    setCurrentPage(page);
    const int n = pageCount();
    if (npages != n)
        Q_EMIT pageCountChanged(n);

    return retval;
}

void GridPageLayout::setExpandingDirections(Qt::Orientations directoins)
{
    if (d->expandingDirections == directoins)
        return;

    d->expandingDirections = directoins;
    invalidate();
}

Qt::Orientations GridPageLayout::expandingDirections() const
{
    return d->expandingDirections;
}

bool GridPageLayout::hasHeightForWidth() const
{
    return d->aspectRatio != 0.0;
}

int GridPageLayout::heightForWidth(int width) const
{
    return d->aspectRatio != 0.0 ? (width * d->aspectRatio) : QLayout::heightForWidth(width);
}

void GridPageLayout::setGeometry(const QRect& rect)
{
    if (geometry() == rect)
        return;

    const int n = itemsPerPage();
    const bool isDynamicGrid = d->flowKind == DynamicBoundedGrid || d->flowKind == DynamicUnboundedGrid;
    if (isDynamicGrid)
    {
        // It's important NOT to use contentsRect() here, since at first
        // run layout can't evaluate contents rect correctly. Instead we
        // have to use ajusted rect with content margins removed
        const QRect rc = rect.marginsRemoved(contentsMargins());
        d->currSize = d->adjustGrid(d->items.size(), rc.size(), this);
        const int ipp = itemsPerPage();
        if (n != ipp)
            Q_EMIT pageCountChanged(pageCount());
    }

    const QRect r = d->effectiveRect(rect, rowCount(), columnCount());
    if (sizeConstraint() == SetMinimumSize)
        QLayout::setGeometry(r);
    else
        QLayout::setGeometry(rect);
    d->doLayout(r, d->currentPage);
}

QSize GridPageLayout::maximumSize() const
{
    if (d->cachedMaxSize.isValid())
        return d->cachedMaxSize;

    d->cachedMaxSize = d->findSize(d->currentPage, d->currSize.cols, rowCount(), columnCount(), &QLayoutItem::maximumSize);
    return d->cachedMaxSize;
}

QSize GridPageLayout::minimumSize() const
{
    if (d->cachedMinSize.isValid())
        return d->cachedMinSize;

    const bool isDynamicGrid = d->flowKind == DynamicBoundedGrid || d->flowKind == DynamicUnboundedGrid;

    LayoutInternals::GridSize size{ rowCount(), columnCount() };
    size = size.shrinkedTo(isDynamicGrid ? d->minSize : size);
    const int desired = isDynamicGrid ? d->minSize.cols : d->currSize.cols;
    d->cachedMinSize = d->findSize(d->currentPage, desired, size.rows, size.cols, &QLayoutItem::minimumSize);
    d->cachedMinSize = d->cachedSizeHint.expandedTo(d->boundingSize(d->minSize));

    return d->cachedMinSize;
}

QSize GridPageLayout::sizeHint() const
{
    if (d->cachedSizeHint.isValid())
        return d->cachedSizeHint;

    const bool isDynamicGrid = d->flowKind == DynamicBoundedGrid || d->flowKind == DynamicUnboundedGrid;
    const int cols = isDynamicGrid ? columnCount() : d->currSize.cols;
    d->cachedSizeHint = d->findSize(d->currentPage, cols, rowCount(), columnCount(), &QLayoutItem::sizeHint);
    d->cachedSizeHint = d->cachedSizeHint.expandedTo(minimumSize());
    return d->cachedSizeHint;
}

void GridPageLayout::invalidate()
{
    d->cachedMinSize = QSize{};
    d->cachedMaxSize = QSize{};
    d->cachedSizeHint = QSize{};
    QLayout::invalidate();
}

int GridPageLayout::pageCount() const
{
    const int n = static_cast<int>(d->items.size());
    if (d->currSize.isEmpty() || n == 0)
        return 0; // avoid division by zero

    const int m = static_cast<int>(std::ceil(n / static_cast<double>(d->itemsPerPage())));
    return std::clamp(n, 1, m);
}

int GridPageLayout::currentPage() const
{
    return d->currentPage;
}

void GridPageLayout::setCurrentPage(int pageIndex)
{
    constexpr int kRectCachePrealloc = 32;

    if (d->currentPage == pageIndex || (pageIndex < 0 || pageIndex >= pageCount()))
        return;

    const QRect rect = geometry();
    invalidate();

    // stop any animation
    for (auto* animation : d->animations)
        animation->stop();

    const int ipp = itemsPerPage();
    const auto prevRange = d->makeRange(d->currentPage, ipp);
    const int prevCount = prevRange.size();

    const auto currRange = d->makeRange(pageIndex, ipp);
    const int currCount = currRange.size();

    QVarLengthArray<QRect, kRectCachePrealloc> rectsCache;
    const bool isSameGridSize = prevCount == currCount;
    if (isSameGridSize)
        rectsCache.reserve(currCount);

    int xOffset = (d->animationFeatures & AnimatePageScroll) ? (d->currentPage < pageIndex ? rect.width() : -rect.width()) : 0;

    for (QLayoutItem* item : prevRange)
    {
        if (isSameGridSize)
            rectsCache.push_back(item->geometry().translated(xOffset, 0));

        d->setItemRect(item, {});
    }

    Q_EMIT updateRequired();

    // it's very important to assign new page index exactly at
    // this point, since doLayout() depends on current page index
    d->currentPage = pageIndex;

    if (isSameGridSize)
    {
        auto first = std::next(d->items.begin(), currRange.lower());
        auto last = std::next(d->items.begin(), currRange.upper());
        std::transform(first, last, rectsCache.begin(), first, [this](QLayoutItem* item, const QRect& r)
        {
            d->setItemRect(item, r);
            return item;
        });

        if (sizeConstraint() != SetMinimumSize)
            d->doLayout(rect, pageIndex, false);
    }
    else
    {
        QRect rc = rect;
        rc = rc.marginsRemoved(contentsMargins());
        rc.translate(xOffset, 0);
        d->doLayout(rc, pageIndex, true);
    }

    //activate();

    Q_EMIT currentPageChanged(d->currentPage);
    Q_EMIT updateRequired();
}

void GridPageLayout::nextPage()
{
    if (d->currentPage < 0 || d->currentPage == (pageCount() - 1))
        return;

    int index = d->currentPage;
    setCurrentPage(++index);
}

void GridPageLayout::prevPage()
{
    if (d->currentPage <= 0)
        return;

    int index = d->currentPage;
    setCurrentPage(--index);
}

void GridPageLayout::ensureItemVisible(QLayoutItem* item)
{
    ensureIndexVisible(indexOf(item));
}

void GridPageLayout::ensureIndexVisible(int index)
{
    if (index >= 0 && index < count())
        setCurrentPage(itemPage(index));
}

QAbstractAnimation* GridPageLayout::createAnimation(QLayout* parent, QLayoutItem* item, const QRect& geometry, const QRect& target) const
{
    constexpr const char* kPropertyName = "layoutItem";

    // stop and remove any animations for item, since only one animation can be active at time
    d->removeAnimation(item, kPropertyName);
    QAbstractAnimation* animation = createItemAnimation<&GridPageLayout::updateRequired>(parent, item, geometry, target, animationOptions());
    if (animation)
        d->emplaceAnimation(animation, kPropertyName, item);

    return animation;
}

