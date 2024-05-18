//
//  W A R N I N G
//  -------------
//
// This file is not part of the public API. It exists for the convenience
// of layout classes implementation. This header file may change from version
// to version without notice, or even be removed.
//
// We mean it.
//

#pragma once
#include <QStyleOptionViewItem>

namespace Qt5ExtraInternals
{
    struct QtStyleOptionViewItemExtra : public QStyleOptionViewItem
    {
        QVariant extraData;
        QModelIndex draggedIndex;
        bool renderDragItem = false;

        QtStyleOptionViewItemExtra() = default;
        QtStyleOptionViewItemExtra(const QStyleOptionViewItem& other) :
            QStyleOptionViewItem(other)
        {}
    };
}
