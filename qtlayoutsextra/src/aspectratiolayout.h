#pragma once
#include "animatedlayout.h"

#include <QtLayoutsExtra>

class QTLAYOUTSEXTRA_EXPORT AspectRatioLayout : public AnimatedLayout
{
    Q_OBJECT
    Q_PROPERTY(double aspectRatio READ aspectRatio WRITE setAspectRatio NOTIFY aspectRatioChanged)

public:
    explicit AspectRatioLayout(QWidget* parent = nullptr, double ratio = 1.0);

    ~AspectRatioLayout();

    /*!
     * \brief Setup aspect ratio to keep.
     *
     * \param value aspect ratio value
     */
    void setAspectRatio(double value);

    /*!
     * \brief Return current aspect ratio.
     *
     * \return current aspect ratio value
     */
    double aspectRatio() const;

    /*!
     * \brief Set the layout item to be the specified layout.
     * \warning If layout already contains any item, it will
     * be removed and deleted automatically
     * \note the layout takes ownership of the item
     * \param layout layout to put inside this layout
     * \param align item alignment
     */
    void setLayout(QLayout* layout, Qt::Alignment alignment = {});

    /*!
     * \brief Set the layout item to be the specified widget.
     * \warning If layout already contains any item, it will
     * be removed and deleted automatically
     * \note the layout does NOT takes ownership of the widget
     * \param layout widget to put inside this layout
     * \param align item alignment
     */
    void setWidget(QWidget* widget, Qt::Alignment alignment = {});

    /*!
     * \brief set the layout item to be the specified item.
     * \warning If layout already contains any item, it will
     * be removed and deleted automatically
     * \note the layout takes ownership of the item
     * \param layout item to put inside this layout
     * \param align item alignment
     */
    void setItem(QLayoutItem* item, Qt::Alignment alignment = {});

    // QLayout interface
public:
    int count() const Q_DECL_OVERRIDE;

    QLayoutItem* itemAt(int index) const Q_DECL_OVERRIDE;

    QLayoutItem* takeAt(int) Q_DECL_OVERRIDE;

    Qt::Orientations expandingDirections() const Q_DECL_OVERRIDE;

    bool hasHeightForWidth() const Q_DECL_OVERRIDE;

    int heightForWidth(int width) const Q_DECL_OVERRIDE;

    void setGeometry(const QRect& rect) Q_DECL_OVERRIDE;

    QSize sizeHint() const Q_DECL_OVERRIDE;

    QSize minimumSize() const Q_DECL_OVERRIDE;

    QSize maximumSize() const Q_DECL_OVERRIDE;

protected:
    void addItem(QLayoutItem* item) Q_DECL_OVERRIDE;

    /*!
     * \brief Evaluate the effective rect of the layout.
     *
     * \param rect input bounding rect
     * \param hint layout used as a hint (currently layout of layout item if such exists)
     * \return rect that keep aspect ratio and fully contained inside bounding rect
     */
    QRect effectiveRect(const QRect& rect, QLayout* hint = nullptr) const;

Q_SIGNALS:
    void aspectRatioChanged(double);

private:
    QScopedPointer<class AspectRatioLayoutPrivate> d;
};
