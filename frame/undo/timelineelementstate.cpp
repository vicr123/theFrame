#include "timelineelementstate.h"

#include <elements/element.h>
#include <elements/timelineelement.h>

TimelineElementState::TimelineElementState(TimelineElement* element)
{
    Element* parent = element->parentElement();
    while (parent->getId() != 0) {
        this->elementIds.push(parent->getId());
        parent = parent->parentElement();
    }
    this->rootElement = parent;

    this->property = element->propertyName();
    this->startFrame = element->startFrame();
    this->endFrame = element->endFrame();
    this->startValue = element->startValue();
    this->endValue = element->endValue();
    this->anchorStart = element->startAnchored();
    this->easingCurve = element->easingCurve();
    this->id = element->getId();
}

TimelineElementState::TimelineElementState()
{

}

Element* TimelineElementState::target()
{
    QStack<uint> ids = this->elementIds;
    Element* parent = this->rootElement;
    while (!ids.isEmpty()) {
        parent = parent->childById(ids.pop());
    }
    return parent;
}

bool TimelineElementState::operator==(TimelineElementState other)
{
    return this->property == other.property &&
            this->startFrame == other.startFrame &&
            this->endFrame == other.endFrame &&
            this->startValue == other.startValue &&
            this->endValue == other.endValue &&
            this->easingCurve == other.easingCurve &&
            this->id == other.id;
}

bool TimelineElementState::operator!=(TimelineElementState other)
{
    return !((*this) == other);
}
