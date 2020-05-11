#ifndef TUTORIALSSETTINGS_H
#define TUTORIALSSETTINGS_H

#include <QWidget>

namespace Ui {
    class TutorialsSettings;
}

struct TutorialsSettingsPrivate;
class TutorialsSettings : public QWidget
{
        Q_OBJECT

    public:
        explicit TutorialsSettings(QWidget *parent = nullptr);
        ~TutorialsSettings();

    private slots:
        void on_resetTutorialsButton_clicked();

        void on_showTutorialsCheckbox_toggled(bool checked);

    private:
        Ui::TutorialsSettings *ui;
        TutorialsSettingsPrivate* d;
};

#endif // TUTORIALSSETTINGS_H
