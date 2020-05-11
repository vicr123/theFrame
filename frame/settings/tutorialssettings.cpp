#include "tutorialssettings.h"
#include "ui_tutorialssettings.h"

#include <tsettings.h>

struct TutorialsSettingsPrivate {
    tSettings settings;
};

TutorialsSettings::TutorialsSettings(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TutorialsSettings)
{
    ui->setupUi(this);
    d = new TutorialsSettingsPrivate();

    ui->showTutorialsCheckbox->setChecked(!d->settings.value("Tutorials/DisableTutorials").toBool());
}

TutorialsSettings::~TutorialsSettings()
{
    delete d;
    delete ui;
}

void TutorialsSettings::on_resetTutorialsButton_clicked()
{
    d->settings.setValue("Tutorials/CompletedTutorials", "0");
}

void TutorialsSettings::on_showTutorialsCheckbox_toggled(bool checked)
{
    d->settings.setValue("Tutorials/DisableTutorials", !checked);
}
