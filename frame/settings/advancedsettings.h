#ifndef ADVANCEDSETTINGS_H
#define ADVANCEDSETTINGS_H

#include <QWidget>

namespace Ui {
    class AdvancedSettings;
}

class AdvancedSettings : public QWidget
{
        Q_OBJECT

    public:
        explicit AdvancedSettings(QWidget *parent = nullptr);
        ~AdvancedSettings();

    private slots:
        void on_resetButton_clicked();

    private:
        Ui::AdvancedSettings *ui;
};

#endif // ADVANCEDSETTINGS_H
