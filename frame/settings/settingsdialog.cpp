#include "settingsdialog.h"
#include "ui_settingsdialog.h"

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    ui->titleLabel->setBackButtonShown(true);
    ui->leftWidget->setFixedWidth(SC_DPI(300));
    ui->stackedWidget->setCurrentAnimation(tStackedWidget::Lift);
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::on_titleLabel_backButtonClicked()
{
    this->close();
}

void SettingsDialog::on_listWidget_currentRowChanged(int currentRow)
{
    ui->stackedWidget->setCurrentIndex(currentRow);
}
