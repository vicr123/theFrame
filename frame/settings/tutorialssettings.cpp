#include "tutorialssettings.h"
#include "ui_tutorialssettings.h"

TutorialsSettings::TutorialsSettings(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TutorialsSettings)
{
    ui->setupUi(this);
}

TutorialsSettings::~TutorialsSettings()
{
    delete ui;
}
