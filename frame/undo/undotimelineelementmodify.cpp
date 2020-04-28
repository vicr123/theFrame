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
    TimelineElement* timelineElement = target->timelineElementById(d->before.id);
    timelineElement->setStartFrame(d->before.startFrame);
    timelineElement->setStartValue(d->before.startValue);
    timelineElement->setEndFrame(d->before.endFrame);
    timelineElement->setEndValue(d->before.endValue);
    timelineElement->setEasingCurve(d->before.easingCurve);
}

void UndoTimelineElementModify::redo()
{
    if (d->ignore) {
        d->ignore = false;
        return;
    }

    Element* target = d->after.target();
    TimelineElement* timelineElement = target->timelineElementById(d->after.id);
    timelineElement->setStartFrame(d->after.startFrame);
    timelineElement->setStartValue(d->after.startValue);
    timelineElement->setEndFrame(d->after.endFrame);
    timelineElement->setEndValue(d->after.endValue);
    timelineElement->setEasingCurve(d->after.easingCurve);
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
    if (d->before.target() == other->d->before.target() && d->before.id == other->d->before.id) {
        d->after = other->d->after;
        return true;
    } else {
        return false;
    }
}
