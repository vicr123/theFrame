#include "undonewtimelineelement.h"

#include <elements/element.h>
#include <elements/timelineelement.h>

#include <QDebug>

struct UndoNewTimelineElementPrivate {
    QList<TimelineElementState> states;

    bool ignore = true;
};

UndoNewTimelineElement::UndoNewTimelineElement(QString text, TimelineElementState newTimelineElement) : QUndoCommand(text)
{
    d = new UndoNewTimelineElementPrivate;
    d->states = {newTimelineElement};
}

UndoNewTimelineElement::UndoNewTimelineElement(QString text, QList<TimelineElementState> newTimelineelements) : QUndoCommand(text)
{
    d = new UndoNewTimelineElementPrivate;
    d->states = newTimelineelements;
}

UndoNewTimelineElement::~UndoNewTimelineElement()
{
    delete d;
}

void UndoNewTimelineElement::undo()
{
    for (auto i = d->states.rbegin(); i != d->states.rend(); i++) {
        i->target()->timelineElementById(i->elementId())->deleteLater();
    }
}

void UndoNewTimelineElement::redo()
{
    if (d->ignore) {
        d->ignore = false;
        return;
    }

    for (TimelineElementState state : d->states) {
        Element* target = state.target();
        TimelineElement* element = new TimelineElement(target);
        element->load(state.data);
        target->addTimelineElement(state.elementProperty(), element, state.elementId());
    }
}
