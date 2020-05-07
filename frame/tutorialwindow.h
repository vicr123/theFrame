#ifndef TUTORIALWINDOW_H
#define TUTORIALWINDOW_H

#include <QWidget>

namespace Ui {
    class TutorialWindow;
}

struct TutorialWindowPrivate;
class TutorialWindow : public QWidget
{
        Q_OBJECT

    public:
        ~TutorialWindow();

        enum TutorialType {
            Idle,
            AddElement,
            AddTimelineElement,
            ChangeProperty,
            ChangeEasing
        };

        enum PopoverSide {
            Horizontal,
            Vertical
        };

        static void trigger(TutorialType type, PopoverSide side, QWidget* widget);
        static void hide(TutorialType type);

    private slots:
        void on_okButton_clicked();

        void on_disableTutorialsButton_clicked();

    private:
        explicit TutorialWindow();
        Ui::TutorialWindow *ui;

        static TutorialWindow* instance();
        TutorialWindowPrivate* d;

        void show(QWidget* widget, PopoverSide side);

        bool eventFilter(QObject* watched, QEvent* event);
        void paintEvent(QPaintEvent *event);
};

#endif // TUTORIALWINDOW_H
