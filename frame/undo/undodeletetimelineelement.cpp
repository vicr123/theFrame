#include "undodeletetimelineelement.h"

#include <elements/element.h>
#include <elements/timelineelement.h>

struct UndoDeleteTimelineElementPrivate {
    TimelineElementState state;

    bool ignore = true;
};

UndoDeleteTimelineElement::UndoDeleteTimelineElement(QString text, TimelineElementState oldTimelineElement) : QUndoCommand(text)
{
    d = new UndoDeleteTimelineElementPrivate;
    d->state = oldTimelineElement;
}

UndoDeleteTimelineElement::~UndoDeleteTimelineElement()
{
    delete d;
}

void UndoDeleteTimelineElement::undo()
{
    Element* target = d->state.target();
    TimelineElement* element = new TimelineElement();
    element->setStartFrame(d->state.startFrame);
    element->setEndFrame(d->state.endFrame);
    element->setStartValue(d->state.startValue);
    element->setEndValue(d->state.endValue);
    element->setEasingCurve(d->state.easingCurve);
    target->addTimelineElement(d->state.property, element, d->state.id);
}

void UndoDeleteTimelineElement::redo()
{
    if (d->ignore) {
        d->ignore = false;
        return;
    }
    d->state.target()->timelineElementById(d->state.id)->deleteLater();
}
