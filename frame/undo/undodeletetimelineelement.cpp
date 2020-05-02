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
    TimelineElement* element = new TimelineElement(target);
    element->load(d->state.data);
    target->addTimelineElement(d->state.elementProperty(), element, d->state.elementId());
}

void UndoDeleteTimelineElement::redo()
{
    if (d->ignore) {
        d->ignore = false;
        return;
    }
    d->state.target()->timelineElementById(d->state.elementId())->deleteLater();
}
