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

    //QtAspectRatioLayout* mainLayout = new QtAspectRatioLayout(this);
    //mainLayout->setLayout(layout);

    screenLayout = new QtScreenLayout;
    screenLayout->setAnimated(true);
    screenLayout->setMinimizationMargins({});
    screenLayout->setSpacing(16);
    screenLayout->setOrientation(Qt::Horizontal);
    screenLayout->setScreenMode(QtScreenLayout::FullGeometry);
    screenLayout->setLayoutMode(QtScreenLayout::GridMode);
}

void Controller::createDialog()
{
    Dialog* d = new Dialog(dialogId++);
    //connect(d, &Dialog::destroyed, []() { dialogId--; });
    d->setFixedSize(256, 328);
    screenLayout->appendWidget(d);
    d->show();
}

Dialog::Dialog(int i) : QDialog(Q_NULLPTR, Qt::Dialog|Qt::FramelessWindowHint)
{
    setAttribute(Qt::WA_DeleteOnClose);

    QLabel* label = new QLabel(tr("Dialog %1").arg(i), this);
    QToolButton* closeBtn = new QToolButton(this);
    connect(closeBtn, &QToolButton::clicked, this, &QDialog::close);
    closeBtn->setToolTip("Close");
    closeBtn->setIcon(QIcon::fromTheme("window-close"));

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->addWidget(label, 1, Qt::AlignTop);
    layout->addWidget(closeBtn, 0, Qt::AlignTop);
}
