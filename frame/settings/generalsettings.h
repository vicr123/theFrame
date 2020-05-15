#ifndef GENERALSETTINGS_H
#define GENERALSETTINGS_H

#include <QWidget>

namespace Ui {
    class GeneralSettings;
}

struct GeneralSettingsPrivate;
class GeneralSettings : public QWidget
{
        Q_OBJECT

    public:
        explicit GeneralSettings(QWidget *parent = nullptr);
        ~GeneralSettings();

    private slots:
        void on_lightThemeRadioButton_toggled(bool checked);

        void on_darkThemeRadioButton_toggled(bool checked);

    private:
        Ui::GeneralSettings *ui;
        GeneralSettingsPrivate* d;

        void updateTheme();
};

#endif // GENERALSETTINGS_H
