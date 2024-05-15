#pragma once
#include <QtWidgets>

class SpellCheckSettings : public QDialog
{
    Q_OBJECT
public:
    explicit SpellCheckSettings(QWidget* parent = nullptr);

private:
    void accept() Q_DECL_OVERRIDE;

private:
    QComboBox* backendsBox;
    QComboBox* detectorsBox;
    QDialogButtonBox* buttonBox;
};
