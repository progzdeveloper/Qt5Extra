#include "qtaboutdialog.h"

#include <QtWidgets/QApplication>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QLabel>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QBoxLayout>

#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtGui/QScreen>
#include <QtGui/QAbstractTextDocumentLayout>

class QtAboutDialogPrivate
{
    Q_DECLARE_TR_FUNCTIONS(QtAboutDialogPrivate)
public:
    QTextBrowser *versionBrowser;
    QTextBrowser *licenseBrowser;
    QTextBrowser *aboutBrowser;
    QDialogButtonBox *buttonBox;
    QTabWidget *infoTabs;
    QLabel *logoLabel;
    QLabel *copyrightLabel;

    void initUi(QWidget *parent);
    void updateSize(QWidget* parent);
};


void QtAboutDialogPrivate::initUi(QWidget *parent)
{
    versionBrowser = new QTextBrowser(parent);
    versionBrowser->setWordWrapMode(QTextOption::NoWrap);
    versionBrowser->setOpenExternalLinks(true);
    versionBrowser->setOpenLinks(true);

    licenseBrowser = new QTextBrowser(parent);
    licenseBrowser->setWordWrapMode(QTextOption::NoWrap);
    licenseBrowser->setOpenExternalLinks(true);
    licenseBrowser->setOpenLinks(true);

    aboutBrowser = new QTextBrowser(parent);
    aboutBrowser->setWordWrapMode(QTextOption::NoWrap);
    aboutBrowser->setOpenExternalLinks(true);
    aboutBrowser->setOpenLinks(true);

    infoTabs = new QTabWidget(parent);
    infoTabs->setDocumentMode(true);
    infoTabs->addTab(aboutBrowser, tr("About"));
    infoTabs->addTab(versionBrowser, tr("Version"));
    infoTabs->addTab(licenseBrowser, tr("License"));

    logoLabel = new QLabel(parent);
    copyrightLabel = new QLabel(parent);

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, Qt::Horizontal, parent);

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(logoLabel);
    layout->addWidget(infoTabs);

    QHBoxLayout* buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(copyrightLabel);
    buttonLayout->addStretch();
    buttonLayout->addWidget(buttonBox);

    QVBoxLayout *mainLayout = new QVBoxLayout(parent);
    mainLayout->addLayout(layout);
    mainLayout->addLayout(buttonLayout);
}

void QtAboutDialogPrivate::updateSize(QWidget * parent)
{
    QAbstractTextDocumentLayout* layout = licenseBrowser->document()->documentLayout();
    QSize documentSize = layout->documentSize().toSize();
    parent->resize(parent->height(), logoLabel->width() + documentSize.width());
}


QtAboutDialog::QtAboutDialog(QWidget *parent)
    : QDialog(parent)
    , d(new QtAboutDialogPrivate)
{
    d->initUi(this);
    connect(d->buttonBox, &QDialogButtonBox::accepted, this, &QtAboutDialog::accept);
    connect(d->buttonBox, &QDialogButtonBox::rejected, this, &QtAboutDialog::reject);
}

QtAboutDialog::QtAboutDialog(const QJsonDocument &json, QWidget *parent)
    : QtAboutDialog(parent)
{
    setJson(json);
}

QtAboutDialog::~QtAboutDialog() = default;

void QtAboutDialog::setJson(const QJsonDocument &document)
{
    setJson(document.object());
}

void QtAboutDialog::setJson(const QJsonObject &json)
{
     
    d->aboutBrowser->setText(json["description"].toString());
    d->versionBrowser->setText(json["version"].toString());
    d->copyrightLabel->setText(json["copyright"].toString());
    QString pixmapPath = json["image"].toString();
    if (!pixmapPath.isEmpty())
    {
        QPixmap pixmap;
        if (pixmap.load(pixmapPath))
            d->logoLabel->setPixmap(pixmap);
    }
    QString licence = json["license"].toString();
    QFileInfo fileInfo(licence);
    if (fileInfo.isFile())
        setLicenseFile(licence);
    else
        setLicenseText(licence);
}

void QtAboutDialog::setAboutText(const QString& text)
{
     
    d->aboutBrowser->setPlainText(text);
}

void QtAboutDialog::setVersionInfo(const QString& version)
{
     
    d->versionBrowser->setPlainText(version);
}

void QtAboutDialog::setCopyrightText(const QString& text)
{
     
    d->copyrightLabel->setText(text);
}

void QtAboutDialog::setLicenseText(const QString &text)
{
     
    if (Qt::mightBeRichText(text))
        d->licenseBrowser->setHtml(text);
    else
        d->licenseBrowser->setPlainText(text);
}

void QtAboutDialog::setLicenseFile(const QString& fileName)
{
    QFile file(fileName);
    file.open(QFile::ReadOnly);
    setLicenseText(file.readAll());
}

void QtAboutDialog::setImage(const QPixmap& pixmap)
{
     
    d->logoLabel->setPixmap(pixmap);
}

