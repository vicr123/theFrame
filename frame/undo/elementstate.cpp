#include "elementstate.h"

#include <elements/element.h>

ElementState::ElementState(Element* element)
{
    if (element->parentElement()) {
        Element* parent = element->parentElement();
        this->index = parent->childElements().indexOf(element);
        while (parent->getId() != 0) {
            this->parentIds.push(parent->getId());
            parent = parent->parentElement();
        }
        this->rootElement = parent;
    } else {
        this->rootElement = element;
    }

    this->elementType = element->metaObject();
    this->data = element->save();
}

ElementState::ElementState()
{

}

uint ElementState::elementId()
{
    return this->data.value("id").toString().toUInt();
}

Element* ElementState::parentElement()
{
    if (this->elementId() == 0) return nullptr;

    QStack<uint> ids = this->parentIds;
    Element* parent = this->rootElement;
    while (!ids.isEmpty()) {
        parent = parent->childById(ids.pop());
    }
    return parent;
}
