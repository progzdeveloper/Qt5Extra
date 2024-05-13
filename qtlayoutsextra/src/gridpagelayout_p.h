//
//  W A R N I N G
//  -------------
//
// This file is not part of the public API. It exists for the convenience
// of GridPageLayout.cpp. This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#pragma once
#include "gridpagelayout.h"
//#include "../common.shared/tools/iterator_traits.h"

namespace Qt5ExtraInternals
{
    /*!
     * \brief Span of range of elements in container.
     * \tparam Type of container, that must provide random access iterators
     */
    template<class _Container>
    struct IterSpan
    {
    public:
        using container_type = _Container;
        using iterator = typename container_type::iterator;
        using const_iterator = typename container_type::const_iterator;
        using reference = typename container_type::reference;
        using const_reference = typename container_type::const_reference;
        using iter_category = typename std::iterator_traits<iterator>::iterator_category;

        static_assert(std::is_base_of_v<iter_category, std::random_access_iterator_tag>,
                      "only random access iterators are supported");

        IterSpan() = delete;
        IterSpan(const IterSpan&) noexcept = default;
        IterSpan(IterSpan&&) noexcept = default;
        IterSpan& operator=(const IterSpan&) noexcept = default;
        IterSpan& operator=(IterSpan&&) noexcept = default;

        explicit IterSpan(const container_type& items) noexcept
            : IterSpan(items, 0, items.size())
        {}

        IterSpan(const container_type& items, size_t first, size_t last) noexcept
            : items_(&items)
            , first_(std::clamp(first, size_t(0), items.size()))
            , last_(std::clamp(last, size_t(0), items.size()))
        {
            if (first_ > last_)
                std::swap(first_, last_);
        }

        const_reference operator[](size_t i) const { return *(items_->begin() + (first_ + i)); }

        const_iterator begin() const { return items_->begin() + first_; }
        const_iterator end() const { return items_->begin() + last_; }

        std::ptrdiff_t lower() const noexcept { return static_cast<std::ptrdiff_t>(first_); }
        std::ptrdiff_t upper() const noexcept { return static_cast<std::ptrdiff_t>(last_); }

        size_t size() const noexcept { return last_ - first_; }
        bool empty() const noexcept { return last_ == first_; }

    private:
        const container_type* items_ = nullptr;
        size_t first_ = 0;
        size_t last_ = 0;
    };

    /*!
     * \brief Class GridSize is like QSize, but in terms of rows/columns count.
     */
    struct GridSize
    {
        void resize(int r, int c) noexcept { rows = r, cols = c; }

        int count() const noexcept { return rows * cols; }

        bool isIdentity() const noexcept { return rows == 1 && cols == 1; }
        bool isEmpty() const noexcept { return rows == 0 && cols == 0; }
        bool isValid() const noexcept { return rows > 0 && cols > 0; }

        void transpose() noexcept { std::swap(rows, cols); }

        GridSize transposed() const noexcept
        {
            GridSize result(*this);
            result.transpose();
            return result;
        }

        GridSize expandedTo(const GridSize& other) const noexcept
        {
            GridSize result;
            result.rows = std::max(rows, other.rows);
            result.cols = std::max(cols, other.cols);
            return result;
        }

        GridSize shrinkedTo(const GridSize& other) const noexcept
        {
            GridSize result;
            result.rows = std::min(rows, other.rows);
            result.cols = std::min(cols, other.cols);
            return result;
        }

        void clamp(const GridSize& minSize, const GridSize& maxSize) noexcept
        {
            rows = std::clamp(rows, minSize.rows, maxSize.rows);
            cols = std::clamp(cols, minSize.cols, maxSize.cols);
        }

        GridSize clamped(const GridSize& minSize, const GridSize& maxSize) const noexcept
        {
            GridSize result(*this);
            result.clamp(minSize, maxSize);
            return result;
        }

        bool operator==(const GridSize& other) const noexcept
        {
            return rows == other.rows && cols == other.cols;
        }

        bool operator!=(const GridSize& other) const noexcept
        {
            return !(*this == other);
        }

        int rows = 0;
        int cols = 0;
    };

    /*!
     * \brief Class GridOptions provide basic information and
     * functionality for grid page layout.
     */
    struct GridOptions
    {
        using GridFlow = GridPageLayout::GridFlow;
        using FillMode = GridPageLayout::PageFillMode;
        using AnimationFeature = GridPageLayout::AnimationFeature;
        using AnimationFeatures = GridPageLayout::AnimationFeatures;

        /*!
         * \brief adjustGrid adjust rows and cols of grid to cover
         * maximal area of rectangle with specified layout.
         *
         * \param count number of desired visible items
         * \param size size of bounding rectangle
         * \param spacing spacing between items
         * \return adjusted grid size
         */
        GridSize adjustGrid(size_t count, const QSize& size, GridPageLayout* layout) const;

        /*!
         * \brief Return maximum grid size of minimal cell size
         * that fits to provided rect size.
         *
         * \param size available size
         * \param spacing spacing between items
         * \return grid size that fits to available rect size
         */
        GridSize availableSize(const QSize& size, int spacing) const;

        /*!
         * \brief Return if grid size is fixed
         *
         * \return true if minimal grid size is equal to maximal, otherwise return false.
         */
        bool isFixedSize() const noexcept;

        GridSize minSize{ 1, 1 };
        GridSize maxSize{ 1, 1 };
        GridSize currSize{ 1, 1 };
        QSize minCellSize;
        double aspectRatio = 0.0;
        Qt::Alignment flowAlign = Qt::AlignCenter;
        GridFlow flowKind = GridFlow::StaticBoundedGrid;
        FillMode fillMode = FillMode::RetainFilled;
        Qt::Orientations expandingDirections = Qt::Horizontal | Qt::Vertical;
    };
} // end namespace LayoutsInternal
