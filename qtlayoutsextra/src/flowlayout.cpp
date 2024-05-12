#include "flowlayout.h"
#include <QWidget>


FlowLayout::FlowLayout(QWidget *_parent, int _margin, int _hSpacing, int _vSpacing)
    : QLayout(_parent), hSpace_(_hSpacing), vSpace_(_vSpacing)
{
    setContentsMargins(_margin, _margin, _margin, _margin);
}

FlowLayout::FlowLayout(int _margin, int _hSpacing, int _vSpacing)
    : FlowLayout(nullptr, _margin, _hSpacing, _vSpacing)
{
}

FlowLayout::~FlowLayout()
{
    qDeleteAll(itemList_);
}

void FlowLayout::addItem(QLayoutItem *_item)
{
    itemList_.append(_item);
}

int FlowLayout::horizontalSpacing() const
{
    if (hSpace_ >= 0)
    {
        return hSpace_;
    }
    else
    {
        return smartSpacing(QStyle::PM_LayoutHorizontalSpacing);
    }
}

int FlowLayout::verticalSpacing() const
{
    if (vSpace_ >= 0)
    {
        return vSpace_;
    }
    else
    {
        return smartSpacing(QStyle::PM_LayoutVerticalSpacing);
    }
}

int FlowLayout::count() const
{
    return itemList_.size();
}

QLayoutItem *FlowLayout::itemAt(int _index) const
{
    return itemList_.value(_index);
}

QLayoutItem *FlowLayout::takeAt(int _index)
{
    if (_index >= 0 && _index < itemList_.size())
        return itemList_.takeAt(_index);
    return nullptr;
}

Qt::Orientations FlowLayout::expandingDirections() const
{
    return {};
}

bool FlowLayout::hasHeightForWidth() const
{
    return true;
}

int FlowLayout::heightForWidth(int _width) const
{
    return doLayout(QRect(0, 0, _width, 0), FlowLayout::Target::test);
}

void FlowLayout::setGeometry(const QRect& _rect)
{
    QLayout::setGeometry(_rect);
    doLayout(_rect, FlowLayout::Target::arrange);
}

QSize FlowLayout::sizeHint() const
{
    return minimumSize();
}

QSize FlowLayout::minimumSize() const
{
    QSize size;
    for (auto item : itemList_)
        size = size.expandedTo(item->minimumSize());

    size += QSize(2*margin(), 2*margin());
    return size;
}

int FlowLayout::doLayout(const QRect& _rect, const FlowLayout::Target _target) const
{
    int left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);
    QRect effectiveRect = _rect.adjusted(+left, +top, -right, -bottom);
    int x = effectiveRect.x();
    int y = effectiveRect.y();
    int lineHeight = 0;

    const bool arranging = _target == FlowLayout::Target::arrange;

    using ItemGeo = std::pair<QLayoutItem*, QRect>;
    using ItemsGeoVector = std::vector<ItemGeo>;
    std::vector<ItemsGeoVector> rows;
    if (arranging && !itemList_.isEmpty())
        rows.push_back({});

    for (auto item : itemList_)
    {
        QWidget *wid = item->widget();
        const auto itemSizeHint = item->sizeHint();

        int spaceX = horizontalSpacing();
        if (spaceX == -1)
            spaceX = wid->style()->layoutSpacing(QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Horizontal);
        int spaceY = verticalSpacing();
        if (spaceY == -1)
            spaceY = wid->style()->layoutSpacing(QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Vertical);

        int nextX = x + itemSizeHint.width() + spaceX;
        if (nextX - spaceX > (effectiveRect.right() + 1) && lineHeight > 0)
        {
            x = effectiveRect.x();
            y = y + lineHeight + spaceY;
            nextX = x + itemSizeHint.width() + spaceX;
            lineHeight = 0;

            if (arranging)
                rows.push_back({});
        }

        if (arranging)
            rows.back().push_back({ item, QRect(QPoint(x, y), itemSizeHint) });

        x = nextX;
        lineHeight = qMax(lineHeight, itemSizeHint.height());
    }

    if (arranging)
    {
        auto calcRowWidth = [](auto& r)
        {
            const auto xLeft = r.front().second.left();
            const auto xRight = r.back().second.right() + 1;
            return xRight - xLeft;
        };

        auto maxRowWidth = 0;
        for (const auto& r : rows)
            if (const auto w = calcRowWidth(r); w > maxRowWidth)
                maxRowWidth = w;

        int rowShift = 0;
        if (alignment() & Qt::AlignRight)
            rowShift = effectiveRect.width() - maxRowWidth;
        else if (alignment() & Qt::AlignHCenter)
            rowShift = (effectiveRect.width() - maxRowWidth) / 2;

        for (const auto& r : rows)
        {
            const auto rowWidth = calcRowWidth(r);

            int shift = rowShift;
            if (innerAlignment() & Qt::AlignRight)
                shift += maxRowWidth - rowWidth;
            else if (innerAlignment() & Qt::AlignHCenter)
                shift += (maxRowWidth - rowWidth) / 2;

            for (const auto& [item, geom] : r)
                item->setGeometry(geom.translated(shift, 0));
        }
    }

    return y + lineHeight - _rect.y() + bottom;
}

int FlowLayout::smartSpacing(QStyle::PixelMetric _pm) const
{
    QObject *parent = this->parent();
    if (!parent)
    {
        return -1;
    }
    else if (parent->isWidgetType())
    {
        QWidget *pw = static_cast<QWidget *>(parent);
        return pw->style()->pixelMetric(_pm, 0, pw);
    }
    else
    {
        return static_cast<QLayout *>(parent)->spacing();
    }
}
