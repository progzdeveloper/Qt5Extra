#include "widget.h"
#include <QVBoxLayout>
#include <QtCardWidget>
#include <QtTextLabel>
#include <QVariant>

static const QString text = "Lorem <b>ipsum</b> <i>dolor</i> sit amet, <b>consectetuer</b> adipiscing <u>elit</u>. Aenean commodo ligula eget dolor. Aenean massa. Cum sociis natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Donec quam felis, ultricies nec, pellentesque eu, pretium quis, sem. Nulla consequat massa quis enim. Donec pede justo, fringilla vel, aliquet nec, vulputate eget, arcu. In enim justo, rhoncus ut, imperdiet a, venenatis vitae, justo.";

Widget::Widget(QWidget *parent) : QWidget(parent)
{
    QtCardWidget* userCard = new QtCardWidget(this);

    userCard->setBadgeValue(123);
    userCard->setCardStyle(Qt::ToolButtonTextUnderIcon);
    userCard->setAvatarRoundness(-1);
    userCard->setAvatar(QPixmap{":/images/avatar"});
    userCard->setText("<b>Jennifer Johnson</b>");
    userCard->setComment("Some comment about User");

    QtTextLabel* label = new QtTextLabel(text, this);
    label->setAlignment(Qt::AlignCenter);
    label->setTextAlign(Qt::AlignJustify);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(QMargins{});
    layout->addWidget(userCard);
    layout->addWidget(label);

}

