#include "widget.h"
#include <QVariant>
#include <QSplitter>
#include <QVBoxLayout>

#include <QtCardWidget>
#include <QtTextLabel>
#include <QtPropertyWidget>

#include <QDebug>

static const QString text = "Lorem <b>ipsum</b> <i>dolor</i> sit amet, <b>consectetuer</b> <a href=\"https://www.google.com\">adipiscing</a> <u>elit</u>. Aenean commodo ligula eget dolor. <a href=\"github.com\">Aenean massa.</a> Cum sociis natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Donec quam felis, ultricies nec, pellentesque eu, pretium quis, sem. Nulla consequat massa quis enim. Donec pede justo, fringilla vel, aliquet nec, vulputate eget, arcu. In enim justo, rhoncus ut, imperdiet a, venenatis vitae, <a href =\"https://www.yahoo.com\"><b>justo<b>.</a>";

Widget::Widget(QWidget *parent) : QWidget(parent)
{
    QtCardWidget* userCard = new QtCardWidget(this);

    userCard->setBadgeValue(42);
    userCard->setCardStyle(Qt::ToolButtonTextUnderIcon);
    userCard->setAvatarRoundness(-1);
    userCard->setAvatar(QPixmap{":/images/avatar"});
    userCard->setText("<b><a href=\"http://github.com\">Jennifer Johnson</a></b>");
    userCard->setComment(tr("<i>Some comment about %1</i>").arg(userCard->text()));

    connect(userCard->textLabel(), &QtTextLabel::linkActivated, this, [](const QString& link) { qDebug() << "clicked" << link; });
    connect(userCard->commentLabel(), &QtTextLabel::linkActivated, this, [](const QString& link) { qDebug()  << "clicked" << link; });

    QtTextLabel* label = new QtTextLabel(text, this);
    label->setAlignment(Qt::AlignCenter);
    label->setTextAlign(Qt::AlignJustify);
    label->setWrapMode(QtTextLabel::WrapAnywhere);
    label->setMaxLineCount(0);
    connect(label, &QtTextLabel::linkActivated, this, [](const QString& link) { qDebug() << "clicked" << link; });

    QWidget* content = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(content);
    layout->setContentsMargins(QMargins{});
    layout->addWidget(userCard);
    layout->addWidget(label, 1);
    layout->addStretch();

    QtPropertyWidget* controller = new QtPropertyWidget(this);
    controller->setObject(userCard);

    QSplitter* splitter = new QSplitter(this);
    splitter->addWidget(content);
    splitter->addWidget(controller);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(splitter);
}

