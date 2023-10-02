#include "qtitemviewnavigator.h"
#include <QAbstractItemView>
#include <QAbstractItemModel>
#include <QPointer>

class QtItemViewNavigatorPrivate
{
public:
    QPointer<QAbstractItemView> view;
};

QtItemViewNavigator::QtItemViewNavigator(QWidget *parent)
    : QtRangeNavigator(parent)
    , d(new QtItemViewNavigatorPrivate)
{
}

QtItemViewNavigator::QtItemViewNavigator(QAbstractItemView *view, QWidget *parent)
    : QtItemViewNavigator(parent)
{
    setView(view);
}

QtItemViewNavigator::~QtItemViewNavigator() = default;

void QtItemViewNavigator::setView(QAbstractItemView *view)
{
     

    if (view == Q_NULLPTR)
        return;

    QAbstractItemModel* model = view->model();
    if (model == Q_NULLPTR)
        return;

    d->view = view;
    setRange(0, model->rowCount());
    connect(this, &QtItemViewNavigator::indexChanged, this, &QtItemViewNavigator::moveTo);
    connect(view, &QAbstractItemView::clicked, this, &QtItemViewNavigator::setIndex);
    connect(view->model(), &QAbstractItemModel::rowsInserted, this, &QtItemViewNavigator::updateRange);
    connect(view->model(), &QAbstractItemModel::rowsRemoved, this, &QtItemViewNavigator::updateRange);
}

QAbstractItemView *QtItemViewNavigator::view() const
{
     
    return d->view;
}

void QtItemViewNavigator::moveTo(int row)
{
     
    QAbstractItemModel* model = d->view->model();
    QModelIndex index = model->index(row, 0);
    if (!index.isValid()) {
        return;
    }
    d->view->scrollTo(index);
    QItemSelectionModel* selectionModel = d->view->selectionModel();
    if (selectionModel != Q_NULLPTR)
        selectionModel->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows);
}

void QtItemViewNavigator::setIndex(const QModelIndex &index)
{
    if (index.isValid()) {
        blockSignals(true);
        setCurrent(index.row() + 1);
        blockSignals(false);
    }
}

void QtItemViewNavigator::updateRange()
{
     
    if (d->view == Q_NULLPTR) {
        setRange(0, 0);
        return;
    }

    QAbstractItemModel* model = d->view->model();
    if (model == Q_NULLPTR) {
        setRange(0, 0);
        return;
    }

    setMaximum(model->rowCount());
}
