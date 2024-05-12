//
//  W A R N I N G
//  -------------
//
// This file is not part of the public API. It exists for the convenience
// of qtcolorgrid.cpp. This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#pragma once
#include <QAbstractListModel>
#include <QStyledItemDelegate>
#include "../painting/qtcolorpalette.h"

namespace QtWidgetExtraInternal
{
    class QtColorGridModel :
            public QAbstractListModel
    {
        Q_OBJECT
    public:
        explicit QtColorGridModel(QObject* parent = Q_NULLPTR);
        ~QtColorGridModel() = default;

        void setPalette(const QtColorPalette& palette);
        const QtColorPalette& palette();

        // Basic functionality:
        int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

        void clear();

    private:
        QtColorPalette mPalette;
    };


    class QtColorGridDelegate :
            public QStyledItemDelegate
    {
        Q_OBJECT

    public:
        explicit QtColorGridDelegate(QObject* parent = Q_NULLPTR);
        ~QtColorGridDelegate() = default;

        // QAbstractItemDelegate interface
    public:
        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;
    };

} // end namespace QtWidgetsExtraInternal
