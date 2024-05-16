#pragma once
#include <QtAnimatedLayout>
#include <QtLayoutsExtra>

class QTLAYOUTSEXTRA_EXPORT QtGridPageLayout : public QtAnimatedLayout
{
    Q_OBJECT

    Q_PROPERTY(double aspectRatio READ aspectRatio WRITE setAspectRatio NOTIFY aspectRatioChanged)
    Q_PROPERTY(Qt::Alignment flowAlignment READ flowAlignment WRITE setFlowAlignment NOTIFY flowAlignmentChanged)
    Q_PROPERTY(GridFlow gridFlow READ gridFlow WRITE setGridFlow NOTIFY gridFlowChanged)
    Q_PROPERTY(PageFillMode fillMode READ fillMode WRITE setFillMode NOTIFY fillModeChanged)

    Q_PROPERTY(QSize minCellSize READ minimumCellSize WRITE setMinimumCellSize NOTIFY minimumCellSizeChanged)
    Q_PROPERTY(int minRowCount READ minRowCount WRITE setMinRowCount NOTIFY minRowCountChanged)
    Q_PROPERTY(int maxRowCount READ maxRowCount WRITE setMaxRowCount NOTIFY maxRowCountChanged)

    Q_PROPERTY(int minColumnCount READ minColumnCount WRITE setMinColumnCount NOTIFY minColumnCountChanged)
    Q_PROPERTY(int maxColumnCount READ maxColumnCount WRITE setMaxColumnCount NOTIFY maxColumnCountChanged)

    Q_PROPERTY(bool isFixedSizeGrid READ isFixedSize)

    Q_PROPERTY(int pageCount READ pageCount NOTIFY pageCountChanged)
    Q_PROPERTY(int currentPage READ currentPage WRITE setCurrentPage NOTIFY currentPageChanged)
    Q_PROPERTY(int itemsPerPage READ itemsPerPage)
    Q_PROPERTY(int rowCount READ rowCount)
    Q_PROPERTY(int columnCount READ columnCount)

public:
    /*!
     * \brief The GridFlow enum controls the grid size adjusting behavior
     *
     * \enum GridPageLayout::GridFlow
     * \var StaticBoundedGrid Maximum number of visible rows and cols are static and fixed
     * \var DynamicBoundedGrid Number of visible rows and cols are computed automatically,
     * but can't be more than maximum, and less than minimum
     * \var DynamicUnboundedGrid Number of visible rows and cols are computed automatically,
     * it can't be less than minimum, but can grow infinitely
     */
    enum GridFlow
    {
        StaticBoundedGrid,
        DynamicBoundedGrid,
        DynamicUnboundedGrid
    };
    Q_ENUM(GridFlow)

    /*!
     * \brief The PageFillMode enum controls the remainder item appearance
     *
     * \enum GridPageLayout::PageFillMode
     * \var AllowUnfilled Allow layout to leave empty items slots if page it not fully filled
     * \var RetainFilled Use any suitable space by stretching or center items if page not fully filled
     * \note The stretching or centering remainder items is controlled by the flow alignment property
     */
    enum PageFillMode
    {
        AllowUnfilled,
        RetainFilled
    };
    Q_ENUM(PageFillMode)

    struct AdjustOptions
    {
        int lastRowUnfilledItems = -1;
        int minAdjustingItemCount = 20;
        int itemCountTolerance = 4;
    };

    /*!
     * \brief The AnimationFeature enum controls animation features
     *
     * \enum GridPageLayout::AnimationOption
     * \var NoAnimation Disable any animations
     * \var AnimateItemsReorder Animate reordering items on inserting/removing
     * \var AnimatePageScroll Animate page scrolling on changing current page
     */
    enum AnimationFeature
    {
        NoAnimation = 0,
        AnimateItemsReorder = 1 << 0,
        AnimatePageScroll = 1 << 1
    };
    Q_DECLARE_FLAGS(AnimationFeatures, AnimationFeature)
    Q_FLAG(AnimationFeatures)

    explicit QtGridPageLayout(QWidget* parent = nullptr);

    ~QtGridPageLayout();

    void setAspectRatio(double ratio);
    double aspectRatio() const;

    void setFlowAlignment(Qt::Alignment alignment);
    Qt::Alignment flowAlignment() const;

    void setGridFlow(GridFlow type);
    GridFlow gridFlow() const;

    void setFillMode(PageFillMode mode);
    PageFillMode fillMode() const;

    void setAnimationFeatures(AnimationFeatures options);
    AnimationFeatures animationFeatures() const;

    void setAdjustOptions(const AdjustOptions& options);
    const AdjustOptions& adjustOptions() const;

    int rowCount(int page = -1) const;
    int columnCount(int page = -1) const;

    void setMaxColumnCount(int n);
    int maxColumnCount() const;

    void setMinColumnCount(int n);
    int minColumnCount() const;

    void setMaxRowCount(int n);
    int maxRowCount() const;

    void setMinRowCount(int n);
    int minRowCount() const;

    void setMaxGridSize(int rows, int cols)
    {
        setMaxRowCount(rows);
        setMaxColumnCount(cols);
    }

    void setMinGridSize(int rows, int cols)
    {
        setMinRowCount(rows);
        setMinColumnCount(cols);
    }

    void setFixedGridSize(int rows, int cols)
    {
        setMaxGridSize(rows, cols);
        setMinGridSize(rows, cols);
    }

    bool isFixedSize() const;

    void setMinimumCellSize(const QSize& s);
    QSize minimumCellSize() const;

    /*!
     * \brief Calculate item index based on given (row, column) pair.
     * \note if page is -1 then current page is used, otherwise
     * index of item on specified page is calculated
     * \param row item row
     * \param column item column
     * \param page page number
     * \return index of item in items array or -1 if index out of range
     */
    int itemIndex(int row, int column, int page = -1) const;

    /*!
     * \brief Calculate page index based on given item index.
     *
     * \param index index of item
     * \return page index contained specified item index
     * or -1 if index is out of range
     */
    int itemPage(int index) const;

    /*!
     * \brief Get the current number of items per page.
     *
     * \return number of items per page
     */
    int itemsPerPage() const;

    /*!
     * \brief Determine if there is next grid page available.
     *
     * \return true if next page is available, otherwise return false
     */
    bool hasNext() const
    {
        const int n = pageCount();
        return (n > 0 ? (currentPage() < (n - 1)) : false);
    }
    /*!
     * \brief Determine if there is previous grid page available.
     *
     * \return true if previous page is available, otherwise return false
     */
    bool hasPrev() const { return currentPage() > 0; }

    /*!
     * \brief Get total number of pages.
     *
     * \return total number of pages
     */
    int pageCount() const;
    /*!
     * \brief Get index of current grid page.
     *
     * \return current page index
     */
    int currentPage() const;

    // QLayout interface
public:
    int count() const Q_DECL_OVERRIDE;

    int addWidget(QWidget* widget);

    void addItem(QLayoutItem* item) Q_DECL_OVERRIDE;

    QLayoutItem* itemAt(int i) const Q_DECL_OVERRIDE;

    QLayoutItem* takeAt(int i) Q_DECL_OVERRIDE;

    void setExpandingDirections(Qt::Orientations directoins);
    Qt::Orientations expandingDirections() const Q_DECL_OVERRIDE;

    bool hasHeightForWidth() const Q_DECL_OVERRIDE;
    int heightForWidth(int width) const Q_DECL_OVERRIDE;

    void setGeometry(const QRect& rect) Q_DECL_OVERRIDE;

    QSize maximumSize() const Q_DECL_OVERRIDE;
    QSize minimumSize() const Q_DECL_OVERRIDE;
    QSize sizeHint() const Q_DECL_OVERRIDE;

    void invalidate() Q_DECL_OVERRIDE;

public Q_SLOTS:
    /*!
     * \brief Set the current page index to pageIndex.
     */
    void setCurrentPage(int pageIndex);
    /*!
     * \brief Move to next page if available.
     */
    void nextPage();

    /*!
     * \brief Move to previous page if available.
     */
    void prevPage();

    /*!
     * \brief Ensures that specified item is visible.
     * \details Scrolls the layout pages so that the item is visible inside
     * the layout viewport. Does nothing if item is nullptr or doesn't belong
     * to this layout
     * \param item item to ensure visibility
     */
    void ensureItemVisible(QLayoutItem* item);

    /*!
     * \brief Ensures that item with specified index is visible..
     * \details Scrolls the layout pages so that the item is visible inside
     * the layout viewport. Does nothing if index is out of range
     * \param index index of item to ensure visibility
     */
    void ensureIndexVisible(int index);

Q_SIGNALS:
    void aspectRatioChanged(double);
    void flowAlignmentChanged(Qt::Alignment);
    void gridFlowChanged(QtGridPageLayout::GridFlow);
    void fillModeChanged(QtGridPageLayout::PageFillMode);

    void minimumCellSizeChanged(const QSize&);
    void minRowCountChanged(int);
    void maxRowCountChanged(int);
    void minColumnCountChanged(int);
    void maxColumnCountChanged(int);

    void currentPageChanged(int);
    void pageCountChanged(int);
    void updateRequired();

    // AnimatedLayout interface
protected:
    QAbstractAnimation* createAnimation(QLayout* parent, QLayoutItem* item, const QRect& geometry, const QRect& target) const Q_DECL_OVERRIDE;

private:
    using QtAnimatedLayout::setAnimated;

private:
    friend class QtGridPageLayoutPrivate;
    QScopedPointer<class QtGridPageLayoutPrivate> d;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(QtGridPageLayout::AnimationFeatures)
