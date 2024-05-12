//
//  W A R N I N G
//  -------------
//
// This file is not part of the public API. It exists for the convenience
// of GridPageLayout.cpp and GridPageLayout_p.h This source file may change
// from version to version without notice, or even be removed.
//
// We mean it.
//

#include "gridpagelayout_p.h"
#include <QtGeometryAlgorithms>

#include <map>
#include <vector>
#include <iterator>
#include <algorithm>
#include <cmath>
#include <QDebug>

namespace
{
    double gridArea(const QSize& size, int rows, int cols, GridPageLayout* layout)
    {
        int w = size.width();
        int h = layout->heightForWidth(w / cols) * rows;
        if (h > size.height())
        {
            h = size.height();
            w = h / layout->aspectRatio() * cols / rows;
        }
        return w * h;
    }
}

namespace LayoutInternals
{
    GridSize GridOptions::availableSize(const QSize& size, int spacing) const
    {
        Q_ASSERT(spacing >= 0);
        spacing = std::max(0, spacing);
        const int rows = std::floor(size.height() / double(minCellSize.height() + spacing));
        const int cols = std::floor(size.width() / double(minCellSize.width() + spacing));
        return { rows, cols };
    }

    bool GridOptions::isFixedSize() const noexcept { return minSize == maxSize; }

    GridSize GridOptions::adjustGrid(size_t count, const QSize& size, GridPageLayout* layout) const
    {
        if (isFixedSize() || static_cast<int>(count) <= minSize.count())
            return minSize;

        const int spacing = std::max(0, layout->spacing());

        // if we allow to unbounded grid size
        const bool isSizeUnbounded = flowKind == GridFlow::DynamicUnboundedGrid;
        // if we constrained in horizontal/vertical direction
        const bool isHVBounded = !(expandingDirections & Qt::Horizontal) || !(expandingDirections & Qt::Vertical);
        const GridSize availSize = isSizeUnbounded ?
            availableSize(size, spacing).expandedTo(minSize) :
            availableSize(size, spacing).clamped(minSize, maxSize);

        if ((isSizeUnbounded || isHVBounded) ||
            availSize.count() <= static_cast<int>(count) ||
            qFuzzyIsNull(layout->aspectRatio()))
        {
            return availSize;
        }

        // at this point it's guaranteed that amount of items that we try to arrange
        // is definitely greater than minimum and less than maximum grid size

        const auto& adjustOptions = layout->adjustOptions();
        const int n = static_cast<int>(count);
        const int maximum = std::max(availSize.cols, availSize.rows) + adjustOptions.itemCountTolerance;

        int upper = std::max(minSize.count(), n + adjustOptions.itemCountTolerance); // upper bound of items per page
        int lower = n; // lower bound of items per page
        if (n > adjustOptions.minAdjustingItemCount)
            lower = std::max(minSize.count(), n - adjustOptions.itemCountTolerance);

        GridSize result{ maximum, maximum };
        double area = 0.0; // current accumulated area

        // adjust grid size by brute-forcing all
        // possible row/column count combinations
        for (int i = maximum; i >= 1; --i)
        {
            for (int j = maximum; j >= 1; --j)
            {
                GridSize current{ i, j };

                if (size.height() > size.width())
                    current.transpose(); // swap rows/cols

                current.clamp(minSize, availSize); // clamp between minimal/available size

                // reject current grid size if we've got number of
                // items per page that is out of desired range: this
                // give us opportunity to change grid size smoothly
                if (current.count() < lower || current.count() > upper)
                    continue;

                // calculate grid area occupied by cells taking into account layout parameters
                const double currArea = gridArea(size, current.rows, current.cols, layout);
                if (currArea > area)
                {
                    area = currArea;
                    result = current;
                }
            }
        }

        // find out if we've got non-suitable number of remainder items
        if (adjustOptions.lastRowUnfilledItems > -1 &&
            result.rows > minSize.rows &&
            result.count() > n &&
            (result.count() - n) > adjustOptions.lastRowUnfilledItems)
        {
             // remove row with non-suitable remainder items
            --result.rows;
        }
        return result;
    }

} // end namespace Ui::Internal
