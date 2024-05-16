#pragma once
#include <chrono>
#include <QMargins>
#include <QEasingCurve>
#include <QMetaMethod>

#include <QtLayoutsExtra>

class QLayout;
class QLayoutItem;
class QAbstractAnimation;


enum class LayoutClearPolicy
{
    DeleteWidgets, //!< Delete wrapped widgets on layout clear
    RetainWidgets  //!< Retain wrapped widgets on layout clear
};

struct LayoutItemAnimationOptions
{
    QMargins margins{ 18, 18, 18, 18 };
    QEasingCurve easingCurve = QEasingCurve::InOutQuad;
    std::chrono::milliseconds duration{ 250 };
};

QTLAYOUTSEXTRA_EXPORT QAbstractAnimation* createItemAnimation(QLayout* parent,
                                                              QLayoutItem* item,
                                                              const QRect& geometry,
                                                              const QRect& target,
                                                              const LayoutItemAnimationOptions& options,
                                                              const QMetaMethod& updatingMethod = {});

template<auto _Signal>
QAbstractAnimation* createItemAnimation(QLayout* parent,
                                        QLayoutItem* item,
                                        const QRect& geometry,
                                        const QRect& target,
                                        const LayoutItemAnimationOptions& options)
{
    return createItemAnimation(parent, item, geometry, target, options, QMetaMethod::fromSignal(_Signal));
}


/*!
 * \brief Transfer items from source layout into target layout.
 *
 * \details Transfer items from source layout into target using
 * mask to conditionaly move items from one layout into another.
 * The mask is a combination of QLayoutItem::ControlType flags
 * and used to filter out items that required to be moved. The
 * default mask is 0, which means transfer all items.
 * No item pointers or references are invalidated, only internal
 * pointer reassignments takes place.
 * \note The ownership of added widgets remains the same as when they was added.
 * \note Does nothing if source is null pointer
 * \param source source layout
 * \param target target layout
 * \param mask layout item mask
 */
QTLAYOUTSEXTRA_EXPORT void transferItems(QLayout* source, QLayout* target, quint32 mask = 0);

/*!
 * \brief Swap items from one layout with another.
 *
 * \details Swap items from source layout with target layout.
 * No item pointers or references are invalidated, only internal
 * pointer reassignments takes place.
 * \note does nothing if either source or target are null pointers
 * \param source source layout
 * \param target target layout
 */
QTLAYOUTSEXTRA_EXPORT void swapItems(QLayout* source, QLayout* target);

/*!
 * \brief Clear layout leaving it witout any items.
 *
 * \details Clear layout leaving it witout any items.
 * The layout items get deleted unconditionally.
 * Wrapped widgets get deleted according to LayoutClearPolicy
 * \note does nothing if layout pointer is nullptr
 */
QTLAYOUTSEXTRA_EXPORT void clearLayout(QLayout* layout, LayoutClearPolicy policy = LayoutClearPolicy::RetainWidgets);

/*!
 * \brief Force layout to refresh.
 *
 * \param layout layout to refresh
 */
QTLAYOUTSEXTRA_EXPORT void refreshLayout(QLayout* layout);

/*!
 * \brief Find layout item that contains point pos.
 * \param layout layout to find
 * \param pos pointer position
 * \return if there is an item under point pos
 * return pair with first pointer to QLayoutItem,
 * and second index of item , otherwise return
 * nullptr as first element and -1 as second element.
 */
QTLAYOUTSEXTRA_EXPORT std::pair<QLayoutItem*, int> layoutItemAt(const QLayout* layout, const QPoint& pos);


/*!
 * \brief Class LayoutItemIterator allows to use STL algorithms with QLayout.
 *
 * \details The LayoutItemIterator is adapter to STL iterator that allows
 * to use QLayout with STL algorithms. The LayoutItemIterator models
 * RandomAccessItreator concept. This implies all limitations of random
 * access iterator, i.e. iterator/reference invalidation upon QLayout
 * addItem()/removeItem(). Main advantage of this class is arises when
 * using with STL non-modifying algorithms.
 * \warning Any modifying operation that changes number of items in layout
 * invalidates this iterator
 */
class QTLAYOUTSEXTRA_EXPORT QtLayoutItemIterator
{
public:
    /*! Iterator category type. */
    using iterator_category = std::random_access_iterator_tag;

    /*! Type alias representing "pointed to" by the iterator. */
    using value_type = QLayoutItem*;

    /*! Type alias representing a reference-to-value_type. */
    using reference = QLayoutItem*;

    /*! Type alias representing a pointer-to-value_type. */
    using pointer = QLayoutItem*;

    /*! Type alias representing distance between iterators. */
    using difference_type = std::ptrdiff_t;
    using distance_type = std::ptrdiff_t;

    /*!
     * \brief Constructor.
     *
     * \param layout pointer to layout
     * \param index position of iterator counted as index of item
     * \note You're rarely need this contructor, using static
     * methods begin()/end() is preferred to obtain an iterator
     */
    QtLayoutItemIterator(QLayout* layout, int index = 0);

    // Compiler-generated copy-move ctors are fine.

    /*!
     * \brief Current item index.
     *
     * \return index of item
     */
    int index() const { return idx_; }
    /*!
     * \brief Return if iterator is valid.
     *
     * \return true if iterator is valid, otherwise return false
     */
    bool isValid() const { return layout_ != nullptr && idx_ != -1; }

    /*!
     * \brief Return the source layout pointer.
     *
     * \return source layout pointer
     */
    QLayout* layout() const { return layout_; }

    reference operator*() const;
    reference operator*();
    reference operator->() const;
    reference operator->();

    QtLayoutItemIterator& operator++()
    {
        advance(1);
        return *this;
    }

    QtLayoutItemIterator operator++(int)
    {
        QtLayoutItemIterator tmp(*this);
        advance(1);
        return tmp;
    }

    QtLayoutItemIterator& operator--()
    {
        advance(-1);
        return *this;
    }

    inline QtLayoutItemIterator operator--(int)
    {
        QtLayoutItemIterator tmp(*this);
        advance(-1);
        return tmp;
    }

    QtLayoutItemIterator& operator+= (difference_type n)
    {
        advance(n);
        return (*this);
    }

    QtLayoutItemIterator& operator-= (difference_type n)
    {
        advance(-n);
        return (*this);
    }

    QtLayoutItemIterator operator+ (difference_type n) const
    {
        QtLayoutItemIterator res(*this);
        res += n;
        return (res);
    }

    QtLayoutItemIterator operator- (difference_type n) const
    {
        QtLayoutItemIterator res(*this);
        res -= n;
        return (res);
    }

    friend difference_type operator- (const QtLayoutItemIterator& x,
                                      const QtLayoutItemIterator& y)
    {
        return x.distance(y);
    }

    friend bool operator< (const QtLayoutItemIterator& lhs,
                           const QtLayoutItemIterator& rhs)
    {
        return (lhs.idx_ < rhs.idx_);
    }

    friend bool operator<= (const QtLayoutItemIterator& lhs,
                            const QtLayoutItemIterator& rhs)
    {
        return (lhs.idx_ <= rhs.idx_);
    }

    friend bool operator> (const QtLayoutItemIterator& lhs,
                           const QtLayoutItemIterator& rhs)
    {
        return (lhs.idx_ > rhs.idx_);
    }

    friend bool operator>= (const QtLayoutItemIterator& lhs,
                            const QtLayoutItemIterator& rhs)
    {
        return (lhs.idx_ >= rhs.idx_);
    }

    bool operator== (const QtLayoutItemIterator& _other) const
    {
        return (layout_ == _other.layout_ && idx_ == _other.idx_);
    }

    bool operator!= (const QtLayoutItemIterator& _other) const
    {
        return !((*this) == _other);
    }

    static QtLayoutItemIterator begin(QLayout* layout);
    static QtLayoutItemIterator end(QLayout* layout);

private:
    void advance(difference_type n);
    difference_type distance(const QtLayoutItemIterator& other) const;

private:
    QLayout* layout_ = nullptr;
    int idx_;
};
