#include "undotimelineelementmodify.h"

#include <elements/element.h>
#include <elements/timelineelement.h>

struct UndoTimelineElementModifyPrivate {
     QList<TimelineElementState> before;
     QList<TimelineElementState> after;

     bool ignore = true;
};

UndoTimelineElementModify::UndoTimelineElementModify(QString text, TimelineElementState before, TimelineElementState after) : QUndoCommand(text)
{
    d = new UndoTimelineElementModifyPrivate();
    d->before = {before};
    d->after = {after};
}

UndoTimelineElementModify::UndoTimelineElementModify(QString text, QList<TimelineElementState> before, QList<TimelineElementState> after) : QUndoCommand(text)
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
    for (TimelineElementState element : d->before) {
        Element* target = element.target();
        TimelineElement* timelineElement = target->timelineElementById(element.elementId());
        timelineElement->load(element.data);
    }
}

void UndoTimelineElementModify::redo()
{
    if (d->ignore) {
        d->ignore = false;
        return;
    }

    for (TimelineElementState element : d->after) {
        Element* target = element.target();
        TimelineElement* timelineElement = target->timelineElementById(element.elementId());
        timelineElement->load(element.data);
    }
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

    if (d->before.count() != other->d->before.count()) return false;
    for (int i = 0; i < d->before.count(); i++) {
        TimelineElementState first = d->before.at(i);
        TimelineElementState second = other->d->before.at(i);

        if (first.target() != second.target()) return false;
        if (first.elementId() != second.elementId()) return false;
    }

    d->after = other->d->after;
    return true;
}
