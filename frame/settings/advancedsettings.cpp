#include "advancedsettings.h"
#include "ui_advancedsettings.h"

#include <tapplication.h>
#include <tsettings.h>

AdvancedSettings::AdvancedSettings(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AdvancedSettings)
{
    ui->setupUi(this);

    ui->resetButton->setProperty("type", "destructive");
}

AdvancedSettings::~AdvancedSettings()
{
    delete ui;
}

void AdvancedSettings::on_resetButton_clicked()
{
    tSettings settings;
    settings.clear();
    settings.sync();

    //Restart the app
    tApplication::restart();
}
