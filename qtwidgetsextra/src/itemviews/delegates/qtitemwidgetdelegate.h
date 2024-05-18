#pragma once
#include <QStyledItemDelegate>

#include <QtWidgetsExtra>

class QtItemWidget;

//
// class QtWidgetItemDelegate provides the ability
// to embed custom widgets as elements of QAbstractItemView
//
class QTWIDGETSEXTRA_EXPORT QtItemWidgetDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    enum RenderHint
    {
        RenderDirect, // render directly into view viewport
        RenderCached  // cached rendering
    };

    enum Option
    {
        NoOptions = 0,
        HighlightSelected = 1 << 0, // enable/disable highlighting item background on mouse press
        HighlightHovered = 1 << 1, // enable/disable highlighting item background on mouse hover
        AutoFillBackground = 1 << 2, // enable/disable auto-fill background of items
        StaticContents = 1 << 3, // hint for static contents
        CacheItemPixmap = 1 << 4, // enable/disable pixmap caching
        CustomEventFilter = 1 << 5 // use QObject::eventFilter() instead of QStyleItemDelegate::editorEvent() for item event handling
    };
    Q_DECLARE_FLAGS(Options, Option)
    Q_FLAG(Option)

    explicit QtItemWidgetDelegate(QObject* parent = Q_NULLPTR);
    ~QtItemWidgetDelegate();

    void setOptions(Options options);
    Options options() const;

    void setCacheLimit(int cacheSize);
    int cacheLimit() const;

    bool isOverDragArea(const QStyleOptionViewItem &option, const QPoint &p) const;

    void setDragIndex(const QModelIndex& index);
    QModelIndex dragIndex() const;

    // QAbstractItemDelegate interface
public:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) Q_DECL_OVERRIDE;

public Q_SLOTS:
    void invalidate();
    void invalidateIndex(const QModelIndex& index);
    void invalidateRange(const QModelIndex& topLeft, const QModelIndex& bottomRight);

Q_SIGNALS:
    void requestRepaint();

    // QObject interface
protected:
    bool eventFilter(QObject *object, QEvent *event) Q_DECL_OVERRIDE;

protected:
    virtual bool eventHandler(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);
    virtual QtItemWidget* createItemWidget() const = 0;
    virtual void updateWidgetData(const QModelIndex& index, const QStyleOptionViewItem &option) const;
    virtual RenderHint renderHint(const QStyleOptionViewItem &option, const QModelIndex &) const;

    QtItemWidget* widget() const;

protected:
    void createWidgetOnDemand() const;

private:
    friend class QtItemWidgetDelegatePrivate;
    QScopedPointer<class QtItemWidgetDelegatePrivate> d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QtItemWidgetDelegate::Options)

