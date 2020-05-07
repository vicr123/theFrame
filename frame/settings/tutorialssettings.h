#ifndef TUTORIALSSETTINGS_H
#define TUTORIALSSETTINGS_H

#include <QWidget>

namespace Ui {
    class TutorialsSettings;
}

class TutorialsSettings : public QWidget
{
        Q_OBJECT

    public:
        explicit TutorialsSettings(QWidget *parent = nullptr);
        ~TutorialsSettings();

    private:
        Ui::TutorialsSettings *ui;
};

#endif // TUTORIALSSETTINGS_H
