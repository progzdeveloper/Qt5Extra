#include "qtitemwidget.h"
#include <QStyleOptionViewItem>

#include <QAbstractButton>
#include <QEvent>
#include <QMouseEvent>


QtItemWidget::QtItemWidget(QWidget *parent) : QWidget(parent)
{
}

bool QtItemWidget::isMouseOver(QWidget *w, QMouseEvent *e) const
{
    return w->rect().contains(w->mapFromGlobal(e->globalPos()));
}

void QtItemWidget::setWidgetVisible(QWidget *w, bool visible)
{
    if (!isAncestorOf(w))
        return;

    if (w->window()->testAttribute(Qt::WA_DontShowOnScreen))
    {
        w->setAttribute(Qt::WA_WState_Visible, visible);
        w->setAttribute(Qt::WA_WState_Hidden, !visible);
    }
    else
    {
        w->setVisible(visible);
    }
}

void QtItemWidget::setVisible(bool visible)
{
    const QWidgetList children = findChildren<QWidget*>();
    for (auto w : children)
        setWidgetVisible(w, visible);
}

void QtItemWidget::setData(const QModelIndex&, const QStyleOptionViewItem &)
{
    // does nothing by default
}

QSize QtItemWidget::sizeHint(const QModelIndex &, const QStyleOptionViewItem &) const
{
    return QWidget::sizeHint();
}

bool QtItemWidget::viewportEvent(QEvent* e, QWidget*, const QStyleOptionViewItem&)
{
    return QWidget::event(e);
}

bool QtItemWidget::handleEvent(QEvent *e, QWidget *viewport, const QStyleOptionViewItem& option)
{
    // this is an important part: here we
    // handle an event and update sub-widget state
    switch(e->type())
    {
    case QEvent::MouseMove:
        handleMouseMove(static_cast<QMouseEvent*>(e), viewport, option);
        break;
    case QEvent::MouseButtonPress:
        handleMousePress(static_cast<QMouseEvent*>(e), viewport, option);
        break;
    case QEvent::MouseButtonRelease:
        handleMouseRelease(static_cast<QMouseEvent*>(e), viewport, option);
        break;
    default:
        break;
    }

    // send event to virtual method, so derived classes can
    // can implement their own custom event handling
    return viewportEvent(e, viewport, option);
}

void QtItemWidget::handleMousePress(QMouseEvent* event, QWidget*, const QStyleOptionViewItem& option)
{
    const bool isMouseClicked = (event->button() == Qt::LeftButton && event->modifiers() == Qt::NoModifier);
    if (!isMouseClicked)
        return;

    QWidget* pointedWidget = childAt(event->pos() - option.rect.topLeft());
    if (!pointedWidget ||!pointedWidget->isEnabled())
        pointedWidget = nullptr;

    const QList<QAbstractButton*> buttons = findChildren<QAbstractButton*>();
    for (auto button : buttons)
    {
        if (button)
        {
            const bool isActiveBtn = button == pointedWidget;
            button->setDown(isActiveBtn);
            if (isActiveBtn)
                activeButton = button;
        }
    }
}

void QtItemWidget::handleMouseMove(QMouseEvent* event, QWidget* viewport, const QStyleOptionViewItem& option)
{
    QWidget* pointedWidget = childAt(event->pos() - option.rect.topLeft());
    if (!pointedWidget ||!pointedWidget->isEnabled())
        pointedWidget = nullptr;

    const QList<QWidget*> children = findChildren<QWidget*>();
    for (auto child : children)
    {
        if (!child->testAttribute(Qt::WA_WState_Visible) || !child->isEnabled())
            continue; // exclude invisible/disabled widgets

        child->setAttribute(Qt::WA_UnderMouse, child != pointedWidget);
        child->setAttribute(Qt::WA_Hover, child != pointedWidget);
    }

    QCursor cursor(Qt::ArrowCursor);
    if (pointedWidget && pointedWidget->testAttribute(Qt::WA_SetCursor))
        cursor = pointedWidget->cursor();

    if (viewport && viewport->cursor().shape() != cursor.shape())
        viewport->setCursor(cursor);
}

void QtItemWidget::handleMouseRelease(QMouseEvent* event, QWidget*, const QStyleOptionViewItem& option)
{
    QWidget* pointedWidget = childAt(event->pos() - option.rect.topLeft());
    if (!pointedWidget ||!pointedWidget->isEnabled())
        pointedWidget = nullptr;

    const bool isMouseClicked = (event->button() == Qt::LeftButton && event->modifiers() == Qt::NoModifier);

    const QList<QAbstractButton*> buttons = findChildren<QAbstractButton*>();
    for (auto button : buttons)
    {
        if (!button)
            continue;

        const bool isClickable = (isMouseClicked &&
                                  button == pointedWidget &&
                                  button->isDown() &&
                                  button->isEnabled() /*&&
                                  button->testAttribute(Qt::WA_WState_Visible)*/);

        if (isClickable)
            button->click();
        button->setDown(false);
    }
    resetState();
}

void QtItemWidget::clearState()
{
    if (activeButton)
        activeButton->setDown(false);
}

void QtItemWidget::applyState()
{
    if (activeButton)
        activeButton->setDown(true);
}

void QtItemWidget::resetState()
{
    activeButton = nullptr;
}
