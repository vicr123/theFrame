#include "generalsettings.h"
#include "ui_generalsettings.h"

GeneralSettings::GeneralSettings(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GeneralSettings)
{
    ui->setupUi(this);
}

GeneralSettings::~GeneralSettings()
{
    delete ui;
}
