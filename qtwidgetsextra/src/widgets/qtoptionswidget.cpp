#include "qtoptionswidget.h"
#include <QDialogButtonBox>
#include <QListWidget>
#include <QStackedWidget>
#include <QHBoxLayout>

class QtOptionsWidgetPrivate
{
public:
    QListWidget *pageList;
    QStackedWidget *content;
};


QtOptionsWidget::QtOptionsWidget(QWidget *parent /*= 0*/)
    : QWidget(parent)
    , d(new QtOptionsWidgetPrivate)
{
    d->content = new QStackedWidget(this);
    d->pageList = new QListWidget(this);
    d->pageList->setMaximumWidth(150);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(d->pageList);
    layout->addWidget(d->content);
    layout->setMargin(0);

    connect(d->pageList, &QListWidget::currentItemChanged, this, &QtOptionsWidget::changePage);
}

void QtOptionsWidget::setListDelegate(QAbstractItemDelegate *delegate)
{
    d->pageList->setItemDelegate(delegate);
}

QAbstractItemDelegate *QtOptionsWidget::listDelegate() const
{
    return d->pageList->itemDelegate();
}

QtOptionsWidget::~QtOptionsWidget() = default;

int QtOptionsWidget::addWidget(QWidget * widget)
{
    new QListWidgetItem(widget->windowTitle(), d->pageList);
    return d->content->addWidget(widget);
}

int QtOptionsWidget::count() const
{
    return d->content->count();
}

int QtOptionsWidget::currentIndex() const
{
    return d->content->currentIndex();
}

QWidget * QtOptionsWidget::currentWidget() const
{
    return d->content->currentWidget();
}

int QtOptionsWidget::indexOf(QWidget * widget) const
{
    return d->content->indexOf(widget);
}

QWidget * QtOptionsWidget::widget(int index) const
{
    return d->content->widget(index);
}

void QtOptionsWidget::setCurrentIndex(int index)
{
    d->content->setCurrentIndex(index);
    d->pageList->setCurrentRow(index);
}

void QtOptionsWidget::setCurrentWidget(QWidget* widget)
{
    d->content->setCurrentWidget(widget);
    d->pageList->setCurrentRow(d->content->indexOf(widget));
}

void QtOptionsWidget::changePage(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (!current)
        current = previous;

    d->content->setCurrentIndex(d->pageList->row(current));
}
