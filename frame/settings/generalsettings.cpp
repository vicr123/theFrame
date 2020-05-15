#include "generalsettings.h"
#include "ui_generalsettings.h"

#include <tsettings.h>

struct GeneralSettingsPrivate {
    tSettings settings;
};

GeneralSettings::GeneralSettings(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GeneralSettings)
{
    ui->setupUi(this);

    d = new GeneralSettingsPrivate();

    connect(&d->settings, &tSettings::settingChanged, this, [=](QString key, QVariant value) {
        if (key == "Theme/mode") {
            updateTheme();
        }
    });
    updateTheme();

    ui->macosThemeRestartPrompt->setVisible(false);
}

GeneralSettings::~GeneralSettings()
{
    delete d;
    delete ui;
}

void GeneralSettings::on_lightThemeRadioButton_toggled(bool checked)
{
    if (checked) {
        d->settings.setValue("Theme/mode", "light");
#ifdef Q_OS_MAC
        ui->macosThemeRestartPrompt->setVisible(true);
#endif
    }
}

void GeneralSettings::on_darkThemeRadioButton_toggled(bool checked)
{
    if (checked) {
        d->settings.setValue("Theme/mode", "dark");
#ifdef Q_OS_MAC
        ui->macosThemeRestartPrompt->setVisible(true);
#endif
    }
}

void GeneralSettings::updateTheme()
{
    if (d->settings.value("Theme/mode") == "dark") {
        ui->darkThemeRadioButton->setChecked(true);
    } else {
        ui->lightThemeRadioButton->setChecked(true);
    }
}
