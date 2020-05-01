#include "undonewelement.h"

#include <elements/element.h>

struct UndoNewElementPrivate {
    QList<ElementState> states;
    
    bool ignore = true;
};

UndoNewElement::UndoNewElement(QString text, ElementState newElement) : QUndoCommand(text)
{
    d = new UndoNewElementPrivate();
    d->states.append(newElement);
}

UndoNewElement::UndoNewElement(QString text, QList<ElementState> newElementStates) : QUndoCommand(text)
{
    d = new UndoNewElementPrivate();
    d->states.append(newElementStates);
}

UndoNewElement::~UndoNewElement()
{
    delete d;
}


void UndoNewElement::undo()
{
    for (auto i = d->states.rbegin(); i != d->states.rend(); i++) {
        i->parentElement()->childById(i->elementId())->deleteLater();
    }
}

void UndoNewElement::redo()
{
    if (d->ignore) {
        d->ignore = false;
        return;
    }
    
    for (ElementState state : d->states) {
        Element* element = qobject_cast<Element*>(state.elementType->newInstance());
        element->load(state.data);
        state.parentElement()->insertChild(state.index, element, state.elementId());
    }
}
