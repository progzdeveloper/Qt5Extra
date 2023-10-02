#include "qtfloatingtoolbutton.h"
#include <QTimeLine>
#include <QMenu>
#include <QAction>
#include <QActionEvent>
#include <QPainter>
#include <QScreen>
#include <QPointer>
#include <QApplication>
#include <QDesktopWidget>
#include <QScreen>

class QtFloatingToolButtonPrivate
{
public:
    QMargins margins;
    QtFloatingToolButton* q_ptr;
    QWidget* parent;
    QTimeLine* timeLine;
    QPointer<QAction> menuAction;
    QAction *defaultAction;
    Qt::Alignment alignment;
    Qt::ToolButtonStyle buttonStyle;
    bool autoRaise;
    bool expanded;
    bool animated;

    QtFloatingToolButtonPrivate(QtFloatingToolButton* q, QWidget* w) :
        q_ptr(q),
        parent(w),
        timeLine(new QTimeLine(100, q)),
        defaultAction(nullptr),
        alignment(Qt::AlignTop|Qt::AlignLeft),
        buttonStyle(Qt::ToolButtonIconOnly),
        autoRaise(false),
        expanded(false),
        animated(false)
    {
    }

    QRect parentRect() const
    {
        /*QScreen* screen = QApplication::desktop()->widget(q_ptr->mapToGlobal({q_ptr->width()/2,0}));
        QRect cr = parent != nullptr ? parent->contentsRect() : screen->availableRect();

        cr.adjust(margins.left(), margins.top(), -margins.right(), -margins.bottom());

        if (align & Qt::AlignLeft)
            cr.setLeft(cr.left());
        if (align & Qt::AlignRight)
            cr.setRight(cr.right());
        if (align & Qt::AlignTop)
            cr.setTop(cr.top());
        if (align & Qt::AlignBottom)
            cr.setBottom(cr.bottom());

        return cr;*/
        return QRect();
    }

    QRect calculateGeometry() const
    {
        /*static const int spacing = 2;
        QRect r = parentRect();
        QSize size = q_ptr->iconSize();
        if (!expanded)
            return QRect(r.x(), r.y(), size.width(), size.height());
        int w = size.width();
        if (QMenu* m = q_ptr->menu()) {
            int n = m->actions().count();
            int s = (n - 1) * spacing;
            w *= n;
            w += s;
        }*/
        return QRect();
    }
};





QtFloatingToolButton::QtFloatingToolButton(QWidget *parent)
    : QAbstractButton(parent)
    , d(new QtFloatingToolButtonPrivate(this, parent))
{
}

QtFloatingToolButton::~QtFloatingToolButton() = default;

void QtFloatingToolButton::setAutoRaise(bool on)
{
     
    d->autoRaise = on;
}

bool QtFloatingToolButton::isAutoRaise() const
{
     
    return d->autoRaise;
}

void QtFloatingToolButton::setExpanded(bool on)
{
     
    d->expanded = on;
    //resize(calculateSize());
}

bool QtFloatingToolButton::isExpanded() const
{
     
    return d->expanded;
}

void QtFloatingToolButton::setAnimated(bool on)
{
     
    d->animated = on;
}

bool QtFloatingToolButton::isAnimated() const
{
     
    return d->animated;
}

void QtFloatingToolButton::setAlignment(Qt::Alignment align)
{
     
    d->alignment = align;
    updateGeometry();
    if (isVisible())
        update();
}

Qt::Alignment QtFloatingToolButton::alignment() const
{
     
    return d->alignment;
}

void QtFloatingToolButton::setToolButtonStyle(Qt::ToolButtonStyle style)
{
     
    if (d->buttonStyle == style)
        return;
    d->buttonStyle = style;
    updateGeometry();
    if (isVisible())
        update();
}

Qt::ToolButtonStyle QtFloatingToolButton::toolButtonStyle() const
{
     
    return d->buttonStyle;
}

void QtFloatingToolButton::setMargins(const QMargins &margins)
{
     
    d->margins = margins;
    updateGeometry();
    if (isVisible())
        update();
}

QMargins QtFloatingToolButton::margins() const
{
     
    return d->margins;
}

void QtFloatingToolButton::setDefaultAction(QAction *action)
{
     

#if QT_CONFIG(menu)
    //bool hadMenu = false;
    //hadMenu = d->hasMenu();
#endif
    d->defaultAction = action;
    if (!action)
        return;
    if (!actions().contains(action))
        addAction(action);
    setText(action->iconText());
    setIcon(action->icon());
#ifndef QT_NO_TOOLTIP
    setToolTip(action->toolTip());
#endif
#if QT_CONFIG(statustip)
    setStatusTip(action->statusTip());
#endif
#if QT_CONFIG(whatsthis)
    setWhatsThis(action->whatsThis());
#endif
    setCheckable(action->isCheckable());
    setChecked(action->isChecked());
    setEnabled(action->isEnabled());
    setFont(action->font());
}

QAction *QtFloatingToolButton::defaultAction() const
{
     
    return d->defaultAction;
}

void QtFloatingToolButton::setMenu(QMenu *menu)
{
     

    if (d->menuAction == (menu ? menu->menuAction() : 0))
        return;
    if (d->menuAction)
        removeAction(d->menuAction);
    if (menu) {
        d->menuAction = menu->menuAction();
        addAction(d->menuAction);
    } else {
        d->menuAction = 0;
    }
    // changing the menu set may change the size hint, so reset it
    updateGeometry();
    update();
}

QMenu *QtFloatingToolButton::menu() const
{
     
    if (d->menuAction)
        return d->menuAction->menu();
    return nullptr;
}

QSize QtFloatingToolButton::minimumSizeHint() const
{
    return QSize(16, 16);
}

void QtFloatingToolButton::showMenu()
{
    setExpanded(true);
}

bool QtFloatingToolButton::hitButton(const QPoint &pos) const
{
    return QAbstractButton::hitButton(pos);
}

void QtFloatingToolButton::changeEvent(QEvent *event)
{
     
    if (event->type() == QEvent::ParentChange)
    {
        d->parent = parentWidget();
    }
    QAbstractButton::changeEvent(event);
}

bool QtFloatingToolButton::event(QEvent *event)
{
    switch(event->type()) {
    case QEvent::HoverEnter:
        setExpanded(true);
        break;
    case QEvent::HoverLeave:
        setExpanded(false);
        break;
    /*case QEvent::HoverMove:
       if (const QHoverEvent *he = static_cast<const QHoverEvent *>(event))
            d_func()->updateHoverControl(he->pos());
        break;*/
    default:
        break;
    }
    return QAbstractButton::event(event);

}

void QtFloatingToolButton::actionEvent(QActionEvent *event)
{
     
    QAction *action = event->action();
    switch (event->type()) {
    case QEvent::ActionChanged:
        if (action == d->defaultAction)
            setDefaultAction(action); // update button state
        break;
    case QEvent::ActionAdded:
        connect(action, &QAction::triggered, this, &QtFloatingToolButton::actionTriggered);
        break;
    case QEvent::ActionRemoved:
        if (d->defaultAction == action)
            d->defaultAction = 0;
#if QT_CONFIG(menu)
        if (action == d->menuAction)
            d->menuAction = 0;
#endif
        action->disconnect(this);
        break;
    default:
        break;
    }
    QAbstractButton::actionEvent(event);
}

void QtFloatingToolButton::paintEvent(QPaintEvent*)
{
     
    QPainter painter(this);
    if (d->expanded)
    {
        int r = std::min(width(), height());
        painter.drawRoundedRect(rect(), r, r);
    }
    else
    {
        painter.setClipping(true);
        painter.setClipRegion(QRegion(rect(), QRegion::Ellipse));
        painter.drawPixmap(rect().adjusted(0, 0, -1, -1), icon().pixmap(size()));
        painter.setClipping(false);
    }
}

void QtFloatingToolButton::mousePressEvent(QMouseEvent *event)
{

}

void QtFloatingToolButton::mouseReleaseEvent(QMouseEvent *event)
{

}

void QtFloatingToolButton::actionTriggered()
{
    if (QAction* action = qobject_cast<QAction*>(sender()))
        Q_EMIT triggered(action);
}
