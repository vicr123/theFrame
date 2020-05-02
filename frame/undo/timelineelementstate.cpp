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

    this->data = element->save();
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

QString TimelineElementState::elementProperty()
{
    return data.value("property").toString();
}

uint TimelineElementState::elementId()
{
    return data.value("id").toString().toUInt();
}

bool TimelineElementState::operator==(TimelineElementState other)
{
    return this->data == other.data;
}

bool TimelineElementState::operator!=(TimelineElementState other)
{
    return !((*this) == other);
}
