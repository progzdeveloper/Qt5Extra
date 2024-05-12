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
#include <QLayout>

class QWidget;

namespace LayoutInternals
{
    class LayoutAssistant
    {
        QLayout* layout_ = nullptr;

    public:
        LayoutAssistant(QLayout* layout)
            : layout_(layout)
        {}

        /*!
         * \brief Check if widget can be added into layout.
         *
         * \param layout layout to add widget
         * \param widget widget to insert
         * \return true if widget can be added to layout, otherwise return false
         */
        bool checkWidget(QWidget* widget) const;

        /*!
         * \brief Check if widget can be added into layout.
         *
         * \param layout layout to add widget
         * \param widget widget to insert
         * \return true if widget can be added to layout, otherwise return false
         */
        bool checkLayout(QLayout* target) const;

        /*!
         * \brief Check if layout item can be added into layout.
         */
        bool checkItem(QLayoutItem* item) const;

        QWidgetItem* createWidgetItem(QWidget* widget, Qt::Alignment alignment = {}) const;
    };
}
