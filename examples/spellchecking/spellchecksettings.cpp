#include "spellchecksettings.h"

#include <QtSpellCheckEngine>
#include <QtSpellCheckBackendFactory>

#include <QtLanguageDetector>
#include <QtLanguageDetectorFactory>

SpellCheckSettings::SpellCheckSettings(QWidget *parent)
{
    backendsBox = new QComboBox(this);
    backendsBox->addItems(QtSpellCheckBackendFactory::instance().keys());
    backendsBox->setCurrentIndex(backendsBox->findData(QtSpellCheckEngine::instance().backendName(), Qt::DisplayRole));

    detectorsBox = new QComboBox(this);
    detectorsBox->addItems(QtLanguageDetectorFactory::instance().keys());
    detectorsBox->setCurrentIndex(detectorsBox->findData(QtLanguageDetector::preferred(), Qt::DisplayRole));

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel, Qt::Horizontal, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QFormLayout* formLayout = new QFormLayout;
    formLayout->addRow(tr("Spelling Backend:"), backendsBox);
    formLayout->addRow(tr("Language Detector:"), detectorsBox);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(buttonBox);
}

void SpellCheckSettings::accept()
{
    QSettings settings;
    settings.setValue("spellchecking/langdetect", detectorsBox->currentText());
    settings.setValue("spellchecking/backend", backendsBox->currentText());
    QDialog::accept();
}
