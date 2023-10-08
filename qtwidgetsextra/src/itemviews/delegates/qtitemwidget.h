#ifndef QTITEMWIDGET_H
#define QTITEMWIDGET_H

#include <QWidget>
#include <QtWidgetsExtra>

class QStyleOptionViewItem;

class QTWIDGETSEXTRA_EXPORT QtItemWidget : public QWidget
{
    Q_OBJECT

    friend class QtItemWidgetDelegate;
public:
    explicit QtItemWidget(QWidget* parent);

    virtual void setData(const QModelIndex&, const QStyleOptionViewItem&);
    virtual QSize sizeHint(const QModelIndex&, const QStyleOptionViewItem&) const;
    using QWidget::sizeHint; // for overload resolution

    void setVisible(bool visible) Q_DECL_OVERRIDE;

protected:
    virtual bool viewportEvent(QEvent* e, QWidget*, const QStyleOptionViewItem&);
    virtual bool handleEvent(QEvent* e, QWidget* viewport, const QStyleOptionViewItem& option);

    void setWidgetVisible(QWidget* w, bool visible);
    bool isMouseOver(QWidget* w, QMouseEvent* e) const;

private:
    void handleMousePress(QMouseEvent* event, QWidget* w, const QStyleOptionViewItem&);
    void handleMouseMove(QMouseEvent* event, QWidget* w, const QStyleOptionViewItem&);
    void handleMouseRelease(QMouseEvent* event, QWidget* w, const QStyleOptionViewItem&);
};


#endif // QTITEMWIDGET_H
