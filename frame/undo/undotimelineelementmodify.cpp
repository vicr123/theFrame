#include "undotimelineelementmodify.h"

#include <elements/element.h>
#include <elements/timelineelement.h>

struct UndoTimelineElementModifyPrivate {
     TimelineElementState before;
     TimelineElementState after;

     bool ignore = true;
};

UndoTimelineElementModify::UndoTimelineElementModify(QString text, TimelineElementState before, TimelineElementState after) : QUndoCommand(text)
{
    d = new UndoTimelineElementModifyPrivate();
    d->before = before;
    d->after = after;
}

UndoTimelineElementModify::~UndoTimelineElementModify()
{
    delete d;
}

void UndoTimelineElementModify::undo()
{
    Element* target = d->before.target();
    TimelineElement* timelineElement = target->timelineElementById(d->before.elementId());
    timelineElement->load(d->before.data);
}

void UndoTimelineElementModify::redo()
{
    if (d->ignore) {
        d->ignore = false;
        return;
    }

    Element* target = d->after.target();
    TimelineElement* timelineElement = target->timelineElementById(d->after.elementId());
    timelineElement->load(d->after.data);
}


int UndoTimelineElementModify::id() const
{
    return 1;
}

bool UndoTimelineElementModify::mergeWith(const QUndoCommand* o)
{
    //Check if we can merge
    if (o->id() != this->id()) return false;
    const UndoTimelineElementModify* other = static_cast<const UndoTimelineElementModify*>(o);
    if (d->before.target() == other->d->before.target() && d->before.elementId() == other->d->before.elementId()) {
        d->after = other->d->after;
        return true;
    } else {
        return false;
    }
}
