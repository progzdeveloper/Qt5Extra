#include <QtWidgets>

#include "qtdaterangeedit.h"
#include "qtdaterange.h"


class QtDateSpanEditPrivate
{
    Q_DECLARE_TR_FUNCTIONS(QtDateSpanEditPrivate)
public:
    QtDateSpanEdit* q_ptr;
    QSpinBox* amountEdit;
    QComboBox* spanEdit;

    QtDateSpanEditPrivate(QtDateSpanEdit* q);
    void initUi(QtDateRange* range);
    void setupRange(int span);
};


QtDateSpanEditPrivate::QtDateSpanEditPrivate(QtDateSpanEdit *q) :
    q_ptr(q)
{
}

void QtDateSpanEditPrivate::initUi(QtDateRange* range)
{
    amountEdit = new QSpinBox(q_ptr);
    amountEdit->setValue(range->step());
    QObject::connect(amountEdit, qOverload<int>(&QSpinBox::valueChanged),
                     range, &QtDateRange::setStep);

    spanEdit = new QComboBox(q_ptr);
    spanEdit->addItem(tr("Hours"),     QtDateRange::Hour);
    spanEdit->addItem(tr("Days"),      QtDateRange::Day);
    spanEdit->addItem(tr("Weeks"),     QtDateRange::Week);
    spanEdit->addItem(tr("Months"),    QtDateRange::Month);
    spanEdit->addItem(tr("Quarters"),  QtDateRange::Quarter);
    spanEdit->addItem(tr("Years"),     QtDateRange::Year);
    spanEdit->addItem(tr("Undefined"), QtDateRange::Undefined);
    QObject::connect(spanEdit, qOverload<int>(&QComboBox::currentIndexChanged),
                     q_ptr, &QtDateSpanEdit::indexChanged);
    QObject::connect(q_ptr, &QtDateSpanEdit::spanChanged,
                     range, &QtDateRange::setSpan);

    QHBoxLayout* layout = new QHBoxLayout(q_ptr);
    layout->addWidget(amountEdit, 0);
    layout->addWidget(spanEdit, 1);
    layout->setContentsMargins(0, 0, 0, 0);
}

void QtDateSpanEditPrivate::setupRange(int span)
{
    int currentYear = QDate::currentDate().year();
    int days = QDate(1, 1, 1).daysTo(QDate::currentDate());
    switch (span) {
    case QtDateRange::Hour:
        amountEdit->setRange(-24 * days, -1);
        break;
    case QtDateRange::Day:
        amountEdit->setRange(-days, -1);
        break;
    case QtDateRange::Week:
        amountEdit->setRange(-(days / 7), -1);
        break;
    case QtDateRange::Month:
        amountEdit->setRange(-12 * currentYear, -1);
        break;
    case QtDateRange::Quarter:
        amountEdit->setRange(-3 * currentYear, -1);
        break;
    case QtDateRange::Year:
        amountEdit->setRange(-(currentYear - 1), -1);
        break;
    default:
        break;
    }
    qDebug() << "amountEdit [" << amountEdit->minimum() << ':' << amountEdit->maximum() << ']';
}

QtDateSpanEdit::QtDateSpanEdit(QtDateRange *range, QWidget *parent)
    : QWidget(parent)
    , d(new QtDateSpanEditPrivate(this))
{
    d->initUi(range);
}

QtDateSpanEdit::~QtDateSpanEdit() = default;

int QtDateSpanEdit::span() const
{
    return d->spanEdit->currentData().toInt();
}

int QtDateSpanEdit::amount() const
{
    return d->amountEdit->value();
}

void QtDateSpanEdit::setSpan(int span)
{
    d->spanEdit->setCurrentIndex(d->spanEdit->findData(span));
}

void QtDateSpanEdit::setAmount(int value)
{
    d->amountEdit->setValue(value);
}

void QtDateSpanEdit::indexChanged(int index)
{
    int span = d->spanEdit->itemData(index).toInt();
    d->amountEdit->setVisible(span != QtDateRange::Undefined);
    d->setupRange(span);
    Q_EMIT spanChanged(span);
}


class QtDateRangeEditPrivate
{
    Q_DECLARE_TR_FUNCTIONS(QtDateSpanEditPrivate)
public:
    QtDateRangeEdit* q_ptr;
    QtDateRange* range;
    QtDateSpanEdit* spanEdit;
    QDateTimeEdit* lowerEdit;
    QDateTimeEdit* upperEdit;
    QLabel* intervalLabel;

    QtDateRangeEditPrivate(QtDateRangeEdit* q);
    void initUi();
    void createActions(QLineEdit* edit, quintptr id);
    QDateTimeEdit* fromSender(QObject *sender);
};

QtDateRangeEditPrivate::QtDateRangeEditPrivate(QtDateRangeEdit *q) :
    q_ptr(q)
{
}

void QtDateRangeEditPrivate::initUi()
{
    QString dateTimeFormat = "yyyy-MM-dd hh:mm:ss";
    range = new QtDateRange(q_ptr);

    QObject::connect(range, &QtDateRange::rangeChanged, q_ptr, &QtDateRangeEdit::setupRange);
    QObject::connect(range, &QtDateRange::rangeChanged, q_ptr, &QtDateRangeEdit::rangeChanged);
    QObject::connect(range, &QtDateRange::spanChanged,  q_ptr, &QtDateRangeEdit::setupSpan);

    lowerEdit = new QDateTimeEdit(q_ptr);
    lowerEdit->setDisplayFormat(dateTimeFormat);
    lowerEdit->setCalendarPopup(true);
    lowerEdit->setDateTime(range->lower());
    createActions(lowerEdit->findChild<QLineEdit*>(), reinterpret_cast<quintptr>(lowerEdit));
    QObject::connect(lowerEdit, &QDateTimeEdit::dateTimeChanged, range, &QtDateRange::setLower);


    upperEdit = new QDateTimeEdit(q_ptr);
    upperEdit->setDisplayFormat(dateTimeFormat);
    upperEdit->setCalendarPopup(true);
    upperEdit->setDateTime(range->upper());
    createActions(upperEdit->findChild<QLineEdit*>(), reinterpret_cast<quintptr>(upperEdit));
    QObject::connect(lowerEdit, &QDateTimeEdit::dateTimeChanged, range, &QtDateRange::setUpper);

    intervalLabel = new QLabel(q_ptr);
    intervalLabel->setText(range->intervalText());
    intervalLabel->setWordWrap(true);

    spanEdit = new QtDateSpanEdit(range, q_ptr);
    spanEdit->setSpan(QtDateRange::Day);

    range->setLower(QDateTime(QDate::currentDate().addDays(-1), QTime(0, 0, 0)));
    range->setSpan(QtDateRange::Day);

    QFormLayout* layout = new QFormLayout(q_ptr);
    layout->addRow(tr("Period:"), spanEdit);
    layout->addRow(tr("Lower:"), lowerEdit);
    layout->addRow(tr("Upper:"), upperEdit);
    layout->addRow(tr("Interval:"), intervalLabel);
}

void QtDateRangeEditPrivate::createActions(QLineEdit *edit, quintptr id)
{
    QScopedPointer<QMenu> menu( edit->createStandardContextMenu() );

    QList<QAction*> actions;

    QAction* currentDateAct = new QAction(tr("Current Date..."), edit);
    currentDateAct->setData(id);
    QObject::connect(currentDateAct, &QAction::triggered, q_ptr, &QtDateRangeEdit::setCurrentDate);

    QAction* currentTimeAct = new QAction(tr("Current Time..."), edit);
    currentTimeAct->setData(id);
    QObject::connect(currentTimeAct, &QAction::triggered, q_ptr, &QtDateRangeEdit::setCurrentTime);

    QAction* currentDateTimeAct = new QAction(tr("Current Date/Time..."), edit);
    currentDateTimeAct->setData(id);
    QObject::connect(currentDateTimeAct, &QAction::triggered, q_ptr, &QtDateRangeEdit::setCurrentDateTime);

    QAction* resetTimeAct = new QAction(tr("Reset Time..."), edit);
    resetTimeAct->setData(id);
    QObject::connect(resetTimeAct, &QAction::triggered, q_ptr, &QtDateRangeEdit::resetTime);

    QAction* separatorAct = new QAction(edit);
    separatorAct->setSeparator(true);
    actions << currentDateAct << currentTimeAct << currentDateTimeAct << resetTimeAct << separatorAct;
    actions << menu->actions();
    for (auto it = actions.cbegin(); it != actions.cend(); ++it)
        (*it)->setParent(edit);

    edit->setContextMenuPolicy(Qt::ActionsContextMenu);
    edit->addActions(actions);
}

QDateTimeEdit *QtDateRangeEditPrivate::fromSender(QObject *sender)
{
    QAction* action = qobject_cast<QAction*>(sender);
    if (action == Q_NULLPTR)
        return Q_NULLPTR;

    return reinterpret_cast<QDateTimeEdit*>(action->data().toULongLong());
}


QtDateRangeEdit::QtDateRangeEdit(QWidget *parent)
    : QWidget(parent)
    , d(new QtDateRangeEditPrivate(this))
{
    d->initUi();
}

QtDateRangeEdit::~QtDateRangeEdit() = default;

QDateTime QtDateRangeEdit::lowerDateTime() const
{
    return d->lowerEdit->dateTime();
}

QDateTime QtDateRangeEdit::upperDateTime() const
{
    return d->upperEdit->dateTime();
}

QDate QtDateRangeEdit::lowerDate() const
{
    return d->lowerEdit->date();
}

QDate QtDateRangeEdit::upperDate() const
{
    return d->upperEdit->date();
}

void QtDateRangeEdit::setupRange(const QDateTime &lower, const QDateTime &upper)
{
    if (d->range->span() == QtDateRange::Undefined)
    {
        d->upperEdit->setMinimumDateTime(lower);
        d->lowerEdit->setMaximumDateTime(upper);
    }
    else
    {
        d->lowerEdit->clearMaximumDateTime();
        d->upperEdit->clearMinimumDateTime();
    }

    d->lowerEdit->blockSignals(true);
    d->lowerEdit->setDateTime(lower);
    d->lowerEdit->blockSignals(false);

    d->upperEdit->blockSignals(true);
    d->upperEdit->setDateTime(upper);
    d->upperEdit->blockSignals(false);

    d->intervalLabel->setText(d->range->intervalText());
}

void QtDateRangeEdit::setupSpan(int span)
{
    if (span == QtDateRange::Undefined)
    {
        d->lowerEdit->setMaximumDateTime(d->range->upper());
        d->upperEdit->setMinimumDateTime(d->range->lower());
    }
    else
    {
        d->lowerEdit->clearMaximumDateTime();
        d->upperEdit->clearMinimumDateTime();
    }
}

void QtDateRangeEdit::setCurrentDate()
{
    QDateTimeEdit* edit = d->fromSender(sender());
    if (edit != Q_NULLPTR)
    {
        edit->setDate(QDate::currentDate());
        edit->setTime(QTime(0, 0, 0));
    }
}

void QtDateRangeEdit::setCurrentTime()
{
    QDateTimeEdit* edit = d->fromSender(sender());
    if (edit != Q_NULLPTR)
        edit->setTime(QTime::currentTime());
}

void QtDateRangeEdit::setCurrentDateTime()
{
    QDateTimeEdit* edit = d->fromSender(sender());
    if (edit != Q_NULLPTR)
        edit->setDateTime(QDateTime::currentDateTime());
}

void QtDateRangeEdit::resetTime()
{
    QDateTimeEdit* edit = d->fromSender(sender());
    if (edit != Q_NULLPTR)
        edit->setTime(QTime(0, 0, 0));
}
