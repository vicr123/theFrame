#include "tutorialengine.h"

#include <QMap>
#include <tsettings.h>

struct TutorialEnginePrivate {
    bool enabled = false;
    TutorialEngine::TutorialState state = TutorialEngine::Idle;

    QMap<TutorialEngine::TutorialState, std::function<void()>> activateCallbacks;
    QMap<TutorialEngine::TutorialState, std::function<void()>> hideCallbacks;

    tSettings settings;
};

TutorialEngine::TutorialEngine(QObject *parent) : QObject(parent)
{
    d = new TutorialEnginePrivate();
}

TutorialEngine::~TutorialEngine()
{
    delete d;
}

void TutorialEngine::setTutorialsEnabled(bool enable)
{
    if (!enable) {
        this->setTutorialState(Idle);
    }
    d->enabled = enable;
}

void TutorialEngine::setTutorialState(TutorialEngine::TutorialState state)
{
    if (!d->enabled) return;
    if (d->state == state) return;
    if (d->hideCallbacks.contains(d->state)) d->hideCallbacks.value(d->state)();

    if (state != Idle) {
        if (d->settings.value("Tutorials/DisableTutorials").toBool()) return;

        QStringList completed = d->settings.delimitedList("Tutorials/CompletedTutorials");
        for (QString tutorialId : completed) {
            if (tutorialId.toInt() == state) return; //Don't do anything because we've already seen this one
        }

        completed.append(QString::number(state));
        d->settings.setDelimitedList("Tutorials/CompletedTutorials", completed);
    }

    d->state = state;
    if (d->activateCallbacks.contains(state)) d->activateCallbacks.value(state)();
}

TutorialEngine::TutorialState TutorialEngine::tutorialState()
{
    return d->state;
}

void TutorialEngine::setTutorialTrigger(TutorialEngine::TutorialState state, std::function<void ()> activate, std::function<void ()> hide)
{
    d->activateCallbacks.insert(state, activate);
    d->hideCallbacks.insert(state, hide);
}

void TutorialEngine::clearTrigger(TutorialEngine::TutorialState state)
{
    d->activateCallbacks.remove(state);
    d->hideCallbacks.remove(state);
}
