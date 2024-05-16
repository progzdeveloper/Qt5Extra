#include "controller.h"
#include <QtScreenLayout>
#include <QtAspectRatioLayout>

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
    screenLayout->setDisplacement({24, 24});
    screenLayout->setLayoutMode(QtScreenLayout::StackMode);
}

void Controller::createDialog()
{
    QDialog* d = new QDialog;
    d->setAttribute(Qt::WA_DeleteOnClose);
    d->setFixedSize(148, 168);
    screenLayout->appendWidget(d);
    d->show();
}
