#include "undodeleteelement.h"

#include <elements/element.h>

struct UndoDeleteElementPrivate {
    ElementState state;

    bool ignore = true;
};

UndoDeleteElement::UndoDeleteElement(QString text, ElementState oldElement) : QUndoCommand(text)
{
    d = new UndoDeleteElementPrivate();
    d->state = oldElement;
}

UndoDeleteElement::~UndoDeleteElement()
{
    delete d;
}

void UndoDeleteElement::undo()
{
    Element* element = qobject_cast<Element*>(d->state.elementType->newInstance());
    element->load(d->state.data);
    d->state.parentElement()->insertChild(d->state.index, element, d->state.elementId());
}

void UndoDeleteElement::redo()
{
    if (d->ignore) {
        d->ignore = false;
        return;
    }

    d->state.parentElement()->childById(d->state.elementId())->deleteLater();
}
