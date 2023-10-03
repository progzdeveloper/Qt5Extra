#include "qtnotificationbar.h"
#include <QLabel>
#include <QToolButton>
#include <QHBoxLayout>
#include <QState>
#include <QFinalState>
#include <QStateMachine>
#include <QSignalTransition>
#include <QPropertyAnimation>
#include <QApplication>
#include <QStyle>
#include <QPalette>
#include <QApplication>

class QtNotificationBarPrivate
{
    Q_DECLARE_TR_FUNCTIONS(QtNotificationBarPrivate)
public:
    QtNotificationBar *q_ptr;
    QLabel *iconLabel;
    QLabel *label;
    QToolButton *closeButton;
    bool animated;
    QPropertyAnimation *animation;

    QtNotificationBarPrivate(QtNotificationBar* bar)
        : q_ptr(bar)
    {}

    void initUi();
};

void QtNotificationBarPrivate::initUi()
{
    animated = true;
    animation = new QPropertyAnimation(q_ptr, "size");
    animation->setDuration(500);

    QStyle * style = qApp->style();
    iconLabel = new QLabel(q_ptr);
    iconLabel->setFixedSize(16, 16);
    label = new QLabel(q_ptr);
    label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    closeButton = new QToolButton(q_ptr);
    closeButton->setFixedSize(16, 16);
    closeButton->setAutoRaise(true);
    closeButton->setIcon(style->standardIcon(QStyle::SP_TitleBarCloseButton));
    closeButton->setToolTip(tr("Hide notification"));

    QHBoxLayout *layout = new QHBoxLayout(q_ptr);
    layout->addWidget(iconLabel);
    layout->addWidget(label);
    layout->addWidget(closeButton);
    layout->setMargin(3);

    QObject::connect(closeButton, &QToolButton::clicked, q_ptr, &QtNotificationBar::close);
}


QtNotificationBar::QtNotificationBar(QWidget *parent) 
    : QFrame(parent)
    , d(new QtNotificationBarPrivate(this))
{
    d->initUi();

    QPalette pal = palette();
    QBrush bg = pal.toolTipBase();
    pal.setBrush(QPalette::Window, bg);
    setPalette(pal);

    setFrameStyle(QFrame::Panel | QFrame::Raised);
    //setWindowFlags(Qt::FramelessWindowHint | Qt::ToolTip);
    setAttribute(Qt::WA_WState_ExplicitShowHide);
    setAttribute(Qt::WA_NoSystemBackground);
    setBackgroundRole(QPalette::Window);
    setVisible(false);
}

QtNotificationBar::~QtNotificationBar() = default;

void QtNotificationBar::setAnimated(bool on)
{

    d->animated = on;
}

bool QtNotificationBar::isAnimated() const
{

    return d->animated;
}

void QtNotificationBar::showMessage(const QString& text, const QIcon& icon, bool closeable)
{
    if (icon.isNull())
        d->iconLabel->hide();
    else
        d->iconLabel->setPixmap(icon.pixmap(16, 16));

    d->closeButton->setVisible(closeable);
    d->label->setText(text);
    show();
    if (d->animated)
        animate();

    resize(size());
}

void QtNotificationBar::animate()
{

    d->animation->setStartValue(QSize(width(), 0));
    d->animation->setEndValue(size());
    d->animation->start();
}

void QtNotificationBar::clearMessage()
{

    d->label->setText(QString());
    d->closeButton->setVisible(true);
    hide();
}
