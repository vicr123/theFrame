#ifndef TUTORIALENGINE_H
#define TUTORIALENGINE_H

#include <QObject>
#include <functional>

struct TutorialEnginePrivate;
class TutorialEngine : public QObject
{
        Q_OBJECT
    public:
        explicit TutorialEngine(QObject *parent = nullptr);
        ~TutorialEngine();

        enum TutorialState {
            Idle = 0,
            AddElement,
            AddTimelineElement,
            ChangeProperty,
            ChangeEasing
        };

        void setTutorialsEnabled(bool enable);

        void setTutorialState(TutorialState state);
        TutorialState tutorialState();

        void setTutorialTrigger(TutorialState state, std::function<void()> activate, std::function<void()> hide);
        void clearTrigger(TutorialState state);

    private:
        TutorialEnginePrivate* d;
};

#endif // TUTORIALENGINE_H
