#include "flowlayout.h"
#include "layoutinternals.h"
#include <QWidget>
#include <QVarLengthArray>

class FlowLayoutPrivate :
        public Qt5ExtraInternals::LayoutAssistant
{
public:
    enum class Operation
    {
        Arrange,
        Test
    };

    FlowLayout* q = nullptr;
    QList<QLayoutItem *> itemList;
    int hSpace;
    int vSpace;
    Qt::Alignment innerAlignment = Qt::AlignLeft;

    FlowLayoutPrivate(FlowLayout* layout, int hSpacing, int vSpacing)
        : LayoutAssistant(layout)
        , q(layout)
        , hSpace(hSpacing)
        , vSpace(vSpacing)
    {
    }

    int doLayout(const QRect& rect, const Operation op) const
    {
        using ItemGeo = std::pair<QLayoutItem*, QRect>;
        using ItemsGeoVector = std::vector<ItemGeo>;

        constexpr int kPreallocRows = 16;
        int left, top, right, bottom;
        q->getContentsMargins(&left, &top, &right, &bottom);
        QRect effectiveRect = rect.adjusted(+left, +top, -right, -bottom);
        int x = effectiveRect.x();
        int y = effectiveRect.y();
        int lineHeight = 0;

        const bool arranging = op == Operation::Arrange;

        QVarLengthArray<ItemsGeoVector, kPreallocRows> rows;
        if (arranging && !itemList.isEmpty())
            rows.push_back({});

        for (auto item : itemList)
        {
            QWidget *wid = item->widget();
            const auto itemSizeHint = item->sizeHint();

            int spaceX = q->horizontalSpacing();
            if (spaceX == -1)
                spaceX = wid->style()->layoutSpacing(QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Horizontal);
            int spaceY = q->verticalSpacing();
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
            if (q->alignment() & Qt::AlignRight)
                rowShift = effectiveRect.width() - maxRowWidth;
            else if (q->alignment() & Qt::AlignHCenter)
                rowShift = (effectiveRect.width() - maxRowWidth) / 2;

            for (const auto& r : rows)
            {
                const auto rowWidth = calcRowWidth(r);

                int shift = rowShift;
                if (q->innerAlignment() & Qt::AlignRight)
                    shift += maxRowWidth - rowWidth;
                else if (q->innerAlignment() & Qt::AlignHCenter)
                    shift += (maxRowWidth - rowWidth) / 2;

                for (const auto& [item, geom] : r)
                    item->setGeometry(geom.translated(shift, 0));
            }
        }

        return y + lineHeight - rect.y() + bottom;
    }

    int smartSpacing(QStyle::PixelMetric pm) const
    {
        QObject *parent = q->parent();
        if (!parent)
            return -1;

        else if (parent->isWidgetType())
        {
            QWidget *pw = static_cast<QWidget *>(parent);
            return pw->style()->pixelMetric(pm, 0, pw);
        }
        else
        {
            return static_cast<QLayout *>(parent)->spacing();
        }
    }
};

FlowLayout::FlowLayout(QWidget *parent, int margin, int hSpacing, int vSpacing)
    : QLayout(parent)
    , d(new FlowLayoutPrivate(this, hSpacing, vSpacing))
{
    setContentsMargins(margin, margin, margin, margin);
}

FlowLayout::FlowLayout(int margin, int hSpacing, int vSpacing)
    : FlowLayout(nullptr, margin, hSpacing, vSpacing)
{
}

FlowLayout::~FlowLayout()
{
    qDeleteAll(d->itemList);
}

void FlowLayout::setInnerAlignment(Qt::Alignment horAlign)
{
    invalidate();
    d->innerAlignment = horAlign;
}

Qt::Alignment FlowLayout::innerAlignment() const
{
    return d->innerAlignment;
}

int FlowLayout::horizontalSpacing() const
{
    return (d->hSpace >= 0 ? d->hSpace : d->smartSpacing(QStyle::PM_LayoutHorizontalSpacing));
}

int FlowLayout::verticalSpacing() const
{
    return (d->vSpace >= 0 ? d->vSpace : d->smartSpacing(QStyle::PM_LayoutVerticalSpacing));
}

void FlowLayout::addWidget(QWidget* widget)
{
    if (!d->checkWidget(widget))
        return;

    QLayout::addWidget(widget); // calls addItem() under the hood
}

void FlowLayout::addLayout(QLayout *layout)
{
    addItem(layout);
}

void FlowLayout::addItem(QLayoutItem* item)
{
    if (!d->checkItem(item))
        return;

    if (auto layout = item->layout())
        adoptLayout(layout);

    invalidate();
    d->itemList.append(item);
}

int FlowLayout::count() const
{
    return d->itemList.size();
}

QLayoutItem *FlowLayout::itemAt(int i) const
{
    return d->itemList.value(i, nullptr);
}

QLayoutItem *FlowLayout::takeAt(int i)
{
    invalidate();
    return (i >= 0 && i < d->itemList.size() ? d->itemList.takeAt(i) : Q_NULLPTR);
}

Qt::Orientations FlowLayout::expandingDirections() const
{
    return {};
}

bool FlowLayout::hasHeightForWidth() const
{
    return true;
}

int FlowLayout::heightForWidth(int width) const
{
    return d->doLayout(QRect(0, 0, width, 0), FlowLayoutPrivate::Operation::Test);
}

void FlowLayout::setGeometry(const QRect& rect)
{
    invalidate();
    QLayout::setGeometry(rect);
    d->doLayout(rect, FlowLayoutPrivate::Operation::Arrange);
}

QSize FlowLayout::sizeHint() const
{
    return minimumSize();
}

QSize FlowLayout::minimumSize() const
{
    QSize size;
    for (auto item : qAsConst(d->itemList))
        size = size.expandedTo(item->minimumSize());

    size += { 2 * margin(), 2 * margin() };
    return size;
}

