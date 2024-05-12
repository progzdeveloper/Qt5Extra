#include "layoutinternals.h"
#include <QWidget>
#include <QMetaObject>

namespace LayoutInternals
{
    bool LayoutAssistant::checkWidget(QWidget* widget) const
    {
        if (Q_UNLIKELY(!layout_))
            return false;

        if (Q_UNLIKELY(!widget))
        {
            qWarning("QLayout: Cannot add a null widget to %s/%ls",
                     layout_->metaObject()->className(),
                     qUtf16Printable(layout_->objectName()));
            return false;
        }

        if (Q_UNLIKELY(widget == layout_->parentWidget()))
        {
            qWarning("QLayout: Cannot add parent widget %s/%ls to its child layout %s/%ls",
                     widget->metaObject()->className(), qUtf16Printable(widget->objectName()),
                    layout_->metaObject()->className(), qUtf16Printable(layout_->objectName()));
            return false;
        }
        return true;
    }

    bool LayoutAssistant::checkLayout(QLayout* target) const
    {
        if (Q_UNLIKELY(!layout_))
            return false;

        if (Q_UNLIKELY(!target))
        {
            qWarning("QLayout: Cannot add a null layout to %s/%ls",
                     layout_->metaObject()->className(), qUtf16Printable(layout_->objectName()));
            return false;
        }

        if (Q_UNLIKELY(target == layout_))
        {
            qWarning("QLayout: Cannot add layout %s/%ls to itself",
                     layout_->metaObject()->className(), qUtf16Printable(layout_->objectName()));
            return false;
        }
        return true;
    }

    bool LayoutAssistant::checkItem(QLayoutItem * item) const
    {
        if (Q_UNLIKELY(!layout_))
            return false;

        if (Q_UNLIKELY(!item))
        {
            qWarning("QLayout: Cannot add a null item to %s/%ls",
                     layout_->metaObject()->className(), qUtf16Printable(layout_->objectName()));
            return false;
        }

        if (Q_UNLIKELY(item == layout_))
        {
            qWarning("QLayout: Cannot add layout item %s/%ls to itself",
                layout_->metaObject()->className(), qUtf16Printable(layout_->objectName()));
            return false;
        }

        if (auto layout = item->layout())
            return checkLayout(layout);
        else if (auto widget = item->widget())
            return checkWidget(widget);
        return true;
    }

    QWidgetItem* LayoutAssistant::createWidgetItem(QWidget * widget, Qt::Alignment alignment) const
    {
        QWidgetItem* item = new QWidgetItemV2(widget);
        item->setAlignment(alignment);
        return item;
    }
}
