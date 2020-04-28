#include "undonewelement.h"

#include <elements/element.h>

struct UndoNewElementPrivate {
    ElementState state;
    
    bool ignore = true;
};

UndoNewElement::UndoNewElement(QString text, ElementState newElement) : QUndoCommand(text)
{
    d = new UndoNewElementPrivate();
    d->state = newElement;
}

UndoNewElement::~UndoNewElement()
{
    delete d;
}


void UndoNewElement::undo()
{
    d->state.parentElement()->childById(d->state.elementId())->deleteLater();
}

void UndoNewElement::redo()
{
    if (d->ignore) {
        d->ignore = false;
        return;
    }
    
    Element* element = qobject_cast<Element*>(d->state.elementType->newInstance());
    element->load(d->state.data);
    d->state.parentElement()->addChild(element, d->state.elementId());
}
