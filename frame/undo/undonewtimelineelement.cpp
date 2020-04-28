#include "undonewtimelineelement.h"

#include <elements/element.h>
#include <elements/timelineelement.h>

#include <QDebug>

struct UndoNewTimelineElementPrivate {
    TimelineElementState state;

    bool ignore = true;
};

UndoNewTimelineElement::UndoNewTimelineElement(QString text, TimelineElementState newTimelineElement) : QUndoCommand(text)
{
    d = new UndoNewTimelineElementPrivate;
    d->state = newTimelineElement;
}

UndoNewTimelineElement::~UndoNewTimelineElement()
{
    delete d;
}

void UndoNewTimelineElement::undo()
{
    d->state.target()->timelineElementById(d->state.id)->deleteLater();
}

void UndoNewTimelineElement::redo()
{
    if (d->ignore) {
        d->ignore = false;
        return;
    }

    Element* target = d->state.target();
    TimelineElement* element = new TimelineElement();
    element->setStartFrame(d->state.startFrame);
    element->setEndFrame(d->state.endFrame);
    element->setStartValue(d->state.startValue);
    element->setEndValue(d->state.endValue);
    element->setEasingCurve(d->state.easingCurve);
    target->addTimelineElement(d->state.property, element, d->state.id);
}
