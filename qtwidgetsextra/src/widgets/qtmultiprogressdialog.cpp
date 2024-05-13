#include "qtmultiprogressdialog.h"
#include "qtmessagelogwidget.h"

#include <QGuiApplication>
#include <QClipboard>

#include <QAction>
#include <QLabel>
#include <QProgressBar>
#include <QCheckBox>
#include <QPushButton>
#include <QBoxLayout>



class QtMultiProgressDialogPrivate
{
    Q_DECLARE_TR_FUNCTIONS(QtMultiProgressDialogPrivate)
public:
    QtMultiProgressDialog* q_ptr;

    QLabel* totalLabel;
    QProgressBar* totalBar;
    QLabel* partialLabel;
    QProgressBar* partialBar;
    QPushButton* detailsButton;
    QtMessageLogWidget* messageBox;
    QCheckBox* autoCloseChecker;
    QScopedPointer<QAbstractButton> cancelButton;
    bool wasCanceled;

    QtMultiProgressDialogPrivate(QtMultiProgressDialog* q);

    void initUi();

    QProgressBar* progressBar(QtMultiProgressDialog::ProgressHint hint) const;
    QLabel* label(QtMultiProgressDialog::ProgressHint hint) const;
};

QtMultiProgressDialogPrivate::QtMultiProgressDialogPrivate(QtMultiProgressDialog *q) :
    q_ptr(q), wasCanceled(false)
{
}

void QtMultiProgressDialogPrivate::initUi()
{
    totalLabel = new QLabel(q_ptr);
    totalBar = new QProgressBar(q_ptr);
    QObject::connect(totalBar, &QProgressBar::valueChanged, q_ptr, &QtMultiProgressDialog::progressChanged);

    partialLabel = new QLabel(q_ptr);
    partialBar = new QProgressBar(q_ptr);
    QObject::connect(partialBar, &QProgressBar::valueChanged, q_ptr, &QtMultiProgressDialog::progressChanged);

    autoCloseChecker = new QCheckBox(tr("Close dialog after operation complete"), q_ptr);
    autoCloseChecker->setChecked(false);

    cancelButton.reset(new QPushButton(tr("Cancel"), q_ptr));
    QObject::connect(cancelButton.data(), &QAbstractButton::clicked, q_ptr, &QtMultiProgressDialog::cancel);

    messageBox = new QtMessageLogWidget(q_ptr);
    messageBox->setAlternatingRowColors(false);
    messageBox->setVisible(false);

    QAction* copyAct = new QAction(QIcon::fromTheme("edit-copy"), tr("Copy text..."), messageBox);
    QObject::connect(copyAct, &QAction::triggered, q_ptr, &QtMultiProgressDialog::copyText);
    messageBox->addAction(copyAct);
    messageBox->setContextMenuPolicy(Qt::ActionsContextMenu);

    detailsButton = new QPushButton(tr("Show Details..."), q_ptr);
    detailsButton->setCheckable(true);
    detailsButton->setChecked(false);
    detailsButton->setVisible(false);
    QObject::connect(detailsButton, &QPushButton::toggled, messageBox, &QtMessageLogWidget::setVisible);
    QObject::connect(detailsButton, &QPushButton::toggled, q_ptr, &QWidget::adjustSize);

    QHBoxLayout* detailsLayout = new QHBoxLayout;
    detailsLayout->setMargin(0);
    detailsLayout->addWidget(detailsButton);
    detailsLayout->addStretch();

    QHBoxLayout* buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(autoCloseChecker);
    buttonLayout->addStretch();
    buttonLayout->addWidget(cancelButton.data());

    QVBoxLayout* mainLayout = new QVBoxLayout(q_ptr);
    mainLayout->addWidget(totalLabel);
    mainLayout->addWidget(totalBar);
    mainLayout->addWidget(partialLabel);
    mainLayout->addWidget(partialBar);
    mainLayout->addLayout(detailsLayout);
    mainLayout->addWidget(messageBox, 1);
    mainLayout->addStretch();
    mainLayout->addLayout(buttonLayout);
}

QProgressBar *QtMultiProgressDialogPrivate::progressBar(QtMultiProgressDialog::ProgressHint hint) const
{
    switch (hint) {
    case QtMultiProgressDialog::PartialProgress:
        return partialBar;
    case QtMultiProgressDialog::TotalProgress:
        return totalBar;
    default:
        break;
    }
    return partialBar;
}

QLabel *QtMultiProgressDialogPrivate::label(QtMultiProgressDialog::ProgressHint hint) const
{
    switch (hint) {
    case QtMultiProgressDialog::PartialProgress:
        return partialLabel;
    case QtMultiProgressDialog::TotalProgress:
        return totalLabel;
    default:
        break;
    }
    return partialLabel;
}



QtMultiProgressDialog::QtMultiProgressDialog(QWidget *parent, Qt::WindowFlags flags)
    : QDialog(parent, flags)
    , d(new QtMultiProgressDialogPrivate(this))
{
    d->initUi();
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

QtMultiProgressDialog::~QtMultiProgressDialog() = default;

void QtMultiProgressDialog::setProgress(int value, ProgressHint hint)
{
    d->progressBar(hint)->setValue(value);
}

int QtMultiProgressDialog::progress(QtMultiProgressDialog::ProgressHint hint) const
{
    return d->progressBar(hint)->value();
}

void QtMultiProgressDialog::setRange(int minimum, int maximum, QtMultiProgressDialog::ProgressHint hint)
{
    d->progressBar(hint)->setRange(minimum, maximum);
}

void QtMultiProgressDialog::setMinimum(int minimum, QtMultiProgressDialog::ProgressHint hint)
{
    d->progressBar(hint)->setMinimum(minimum);
}

int QtMultiProgressDialog::minimum(QtMultiProgressDialog::ProgressHint hint) const
{
    return d->progressBar(hint)->minimum();
}

void QtMultiProgressDialog::setMaximum(int maximum, QtMultiProgressDialog::ProgressHint hint)
{
    d->progressBar(hint)->setMaximum(maximum);
}

int QtMultiProgressDialog::maximum(QtMultiProgressDialog::ProgressHint hint) const
{
    return d->progressBar(hint)->maximum();
}

void QtMultiProgressDialog::setLabelText(const QString &text, QtMultiProgressDialog::ProgressHint hint)
{
    d->label(hint)->setText(text);
}

QString QtMultiProgressDialog::labelText(QtMultiProgressDialog::ProgressHint hint) const
{
    return d->label(hint)->text();
}

void QtMultiProgressDialog::setAutoClose(bool on)
{
    d->autoCloseChecker->setChecked(on);
}

bool QtMultiProgressDialog::isAutoClose() const
{
    return d->autoCloseChecker->isChecked();
}

bool QtMultiProgressDialog::wasCanceled() const
{
    return d->wasCanceled;
}

void QtMultiProgressDialog::setCancelButton(QAbstractButton *cancelButton)
{
    if (cancelButton == Q_NULLPTR) {
        d->cancelButton->hide();
    } else {
        d->cancelButton.reset(cancelButton);
    }
}

QAbstractButton *QtMultiProgressDialog::cancelButton() const
{
    return d->cancelButton.data();
}

void QtMultiProgressDialog::cancel()
{
    d->wasCanceled = true;
    Q_EMIT canceled();
    d->messageBox->clear();
}

void QtMultiProgressDialog::reset()
{
    d->wasCanceled = false;
    d->cancelButton->setEnabled(true);
    d->autoCloseChecker->setVisible(true);
    d->messageBox->clear();
    setProgress(0, TotalProgress);
    setProgress(0, PartialProgress);
}

void QtMultiProgressDialog::message(const QString &text)
{
    if (!d->detailsButton->isVisible())
        d->detailsButton->setVisible(true);
    d->messageBox->message(text);
}

void QtMultiProgressDialog::progressChanged(int value)
{
    QProgressBar* progressBar = qobject_cast<QProgressBar*>(sender());
    if (progressBar == d->partialBar && value == d->partialBar->maximum())
        d->totalBar->setValue(d->totalBar->value() + 1);

    if (progressBar == d->totalBar && value == d->totalBar->maximum())
    {
        d->cancelButton->disconnect();
        d->cancelButton->setText(tr("Close"));
        QObject::connect(d->cancelButton.data(), &QAbstractButton::clicked, &QtMultiProgressDialog::close);
        if (d->autoCloseChecker->isChecked())
            close();
        else
            d->autoCloseChecker->setVisible(false);
    }
}

void QtMultiProgressDialog::copyText()
{
    QGuiApplication::clipboard()->setText(d->messageBox->text());
}



