#include "controller.h"
#include <QtScreenLayout>
#include <QtAspectRatioLayout>

static int dialogId = 0;

Controller::Controller(QWidget* parent)
    : QWidget(parent)
{
    button = new QPushButton(tr("Push Me!"), this);
    connect(button, &QPushButton::clicked, this, &Controller::createDialog);
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->addWidget(button);

    screenLayout = new QtScreenLayout;
    screenLayout->setContentsMargins(16, 16, 16, 16);
    screenLayout->setAnimated(true);
    screenLayout->setAnimationDuration(std::chrono::milliseconds{200});
    screenLayout->setMinimizationMargins({});
    screenLayout->setSpacing(16);
    screenLayout->setOrientation(Qt::Vertical);
    screenLayout->setScreenMode(QtScreenLayout::CustomGeometry);
    screenLayout->setLayoutMode(QtScreenLayout::BoxMode);
    screenLayout->setAlignment(Qt::AlignTop|Qt::AlignRight);
    screenLayout->setEnqueueMode(QtScreenLayout::EnqueueFront);
}

void Controller::createDialog()
{
    Dialog* d = new Dialog(dialogId++);
    connect(d, &Dialog::destroyed, this, &Controller::onDestroyed);
    dialogs.insert(d);

    d->setFixedSize(328, 64);
    screenLayout->appendWidget(d);
    QTimer::singleShot(screenLayout->animationDurationAsInt(), d, &Dialog::show);
}

void Controller::onDestroyed(QObject *object)
{
    if (auto w = qobject_cast<QWidget*>(object))
        dialogs.erase(w);
}

void Controller::moveEvent(QMoveEvent *e)
{
    QWidget::moveEvent(e);
    screenLayout->updateGeometry(geometry());
}

void Controller::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
    screenLayout->updateGeometry(geometry());
}

void Controller::showEvent(QShowEvent *e)
{
    QWidget::showEvent(e);
    for (auto* w : dialogs)
        w->show();
}

void Controller::hideEvent(QHideEvent *e)
{
    for (auto* w : dialogs)
        w->hide();

    QWidget::hideEvent(e);
}

Dialog::Dialog(int i) : QDialog(Q_NULLPTR, Qt::ToolTip|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_ShowWithoutActivating);

    QLabel* label = new QLabel(tr("Dialog %1").arg(i), this);
    QToolButton* closeBtn = new QToolButton(this);
    connect(closeBtn, &QToolButton::clicked, this, &QDialog::close);
    closeBtn->setToolTip("Close");
    closeBtn->setIcon(QIcon::fromTheme("window-close"));

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->addWidget(label, 1, Qt::AlignTop);
    layout->addWidget(closeBtn, 0, Qt::AlignTop);
}
