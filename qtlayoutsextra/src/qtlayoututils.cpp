#include "qtlayoututils.h"

#include <QMetaObject>
#include <QVariantAnimation>
#include <QLayout>
#include <QWidget>
#include <QPointer>
#include <deque>
#include <utility>

struct LayoutItemAnimationHandler
{
    const QMetaMethod method;
    QRect layoutGeometry;
    QPointer<QLayout> layout;
    QLayoutItem* item;
    QVariantAnimation* animation;

    LayoutItemAnimationHandler(const QMetaMethod& metaMethod, QLayout* layout, QLayoutItem* item, QVariantAnimation* animation)
        : method(validateMetaMethod(metaMethod, layout) ? metaMethod : QMetaMethod{})
        , layout(layout)
        , item(item)
        , animation(animation)

    {
        if (layout)
            layoutGeometry = layout->geometry();
    }

    void operator()(const QVariant& value) const
    {
        if (!layout || !containsItem()) // layout or item was deleted
        {
            animation->stop();
            return;
        }

        if (layoutGeometry != layout->geometry())
            animation->setEndValue(item->geometry());
        else
            item->setGeometry(value.toRect());

        if (method.isValid())
            method.invoke(layout, Qt::AutoConnection);
    }

    bool containsItem() const
    {
        auto first = QtLayoutItemIterator::begin(layout);
        auto last = QtLayoutItemIterator::end(layout);
        return std::find(first, last, item) != last;
    }

    static bool validateMetaMethod(const QMetaMethod& metaMethod, QObject* object)
    {
        if (!object || !metaMethod.isValid())
            return false;

        return metaMethod.enclosingMetaObject() == object->metaObject() && metaMethod.parameterCount() == 0;
    }
};

QAbstractAnimation* createItemAnimation(QLayout* parent,
                                        QLayoutItem* item,
                                        const QRect& geometry,
                                        const QRect& target,
                                        const LayoutItemAnimationOptions& options,
                                        const QMetaMethod& updatingMethod)
{
    if (!item || !parent)
        return nullptr;

    QRect source = geometry;
    if (source == target)
        return nullptr;

    if (!source.isValid())
        source = target.marginsRemoved(options.margins);

    QVariantAnimation* animation = new QVariantAnimation(parent);
    animation->setDuration(options.duration.count());
    animation->setEasingCurve(options.easingCurve);
    animation->setStartValue(source);
    animation->setEndValue(target);

    LayoutItemAnimationHandler animationHandler(updatingMethod, parent, item, animation);
    QObject::connect(animation, &QVariantAnimation::valueChanged, animationHandler);

    return animation;
}

void transferItems(QLayout* source, QLayout* target, quint32 mask)
{
    if (!source || !target)
        return;

    if (mask == 0) // transfer all items
    {
        while (source->count() > 0)
            target->addItem(source->takeAt(0));
    }
    else
    {
        for (int i = 0; i < source->count(); )
        {
            QLayoutItem* item = source->itemAt(i);
            if (item->controlTypes() & mask)
                target->addItem(source->takeAt(i));
            else
                ++i;
        }
    }
}

void swapItems(QLayout* source, QLayout* target)
{
    if (!source || !target)
        return;

    std::deque<QLayoutItem*> storage;
    while (target->count() > 0)
        storage.push_back(target->takeAt(0));

    transferItems(source, target);

    for (; !storage.empty(); storage.pop_front())
        source->addItem(storage.front());
}

void clearLayout(QLayout* layout, LayoutClearPolicy policy)
{
    if (!layout)
        return;

    while (layout->count() > 0)
    {
        QWidget* widget = nullptr;
        auto item = layout->takeAt(0);
        if (item)
            widget = item->widget();

        if (widget && policy == LayoutClearPolicy::DeleteWidgets)
            widget->deleteLater();

        delete item;
    }
}

void refreshLayout(QLayout* layout)
{
    if (!layout)
        return;

    if (QLayout* parent = qobject_cast<QLayout*>(layout->parent()))
        parent->invalidate();
    else if (QWidget* widget = layout->parentWidget())
        widget->updateGeometry();
}

std::pair<QLayoutItem*, int> layoutItemAt(const QLayout* layout, const QPoint& pos)
{
    auto first = QtLayoutItemIterator::begin(const_cast<QLayout*>(layout));
    auto last = QtLayoutItemIterator::end(const_cast<QLayout*>(layout));
    auto it = std::find_if(first, last, [pos](QLayoutItem* item)
    {
            if (!item)
            return false;
            const QRect rc = item->geometry();
            return rc.isValid() && rc.contains(pos);
});
    return it != last ? std::make_pair(*it, it.index()) : std::make_pair(nullptr, -1);
}


QtLayoutItemIterator::QtLayoutItemIterator(QLayout* layout, int index)
    : layout_(layout)
    , idx_(index)
{
    if (!layout_)
    {
        idx_ = -1;
        return;
    }

    idx_ = std::clamp(idx_, 0, layout_->count());
}

QtLayoutItemIterator::reference QtLayoutItemIterator::operator*()
{
    return layout_ ? layout_->itemAt(idx_) : nullptr;
}

QtLayoutItemIterator::reference QtLayoutItemIterator::operator*() const
{
    return layout_ ? layout_->itemAt(idx_) : nullptr;
}

QtLayoutItemIterator::reference QtLayoutItemIterator::operator->()
{
    return layout_ ? layout_->itemAt(idx_) : nullptr;
}

QtLayoutItemIterator::reference QtLayoutItemIterator::operator->() const
{
    return layout_ ? layout_->itemAt(idx_) : nullptr;
}

void QtLayoutItemIterator::advance(QtLayoutItemIterator::difference_type n)
{
    if (!layout_)
        return;
    idx_ += n;
}

QtLayoutItemIterator::difference_type QtLayoutItemIterator::distance(const QtLayoutItemIterator& other) const
{
    Q_ASSERT(layout_ == other.layout_);
    return idx_ - other.idx_;
}

QtLayoutItemIterator QtLayoutItemIterator::begin(QLayout* layout)
{
    return { layout, 0 };
}

QtLayoutItemIterator QtLayoutItemIterator::end(QLayout* layout)
{
    return { layout, layout->count() };
}
