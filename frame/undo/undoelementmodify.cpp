#include "undoelementmodify.h"

#include <elements/element.h>

struct UndoElementModifyPrivate {
    ElementState before;
    ElementState after;

    bool ignore = true;
};

UndoElementModify::UndoElementModify(QString text, ElementState before, ElementState after) : QUndoCommand(text)
{
    d = new UndoElementModifyPrivate();
    d->before = before;
    d->after = after;
}

UndoElementModify::~UndoElementModify()
{
    delete d;
}

void UndoElementModify::undo()
{
    if (d->before.parentElement() == nullptr) {
        d->before.rootElement->load(d->before.data);
    } else {
        d->before.parentElement()->childById(d->before.elementId())->load(d->before.data);
    }
}

void UndoElementModify::redo()
{
    if (d->ignore) {
        d->ignore = false;
        return;
    }

    if (d->after.parentElement() == nullptr) {
        d->after.rootElement->load(d->after.data);
    } else {
        d->after.parentElement()->childById(d->after.elementId())->load(d->after.data);
    }
}


int UndoElementModify::id() const
{
    return 2;
}

bool UndoElementModify::mergeWith(const QUndoCommand* o)
{
    //Check if we can merge
    if (o->id() != this->id()) return false;
    const UndoElementModify* other = static_cast<const UndoElementModify*>(o);
    if (d->before.parentElement() == other->d->before.parentElement() && d->before.elementId() == other->d->before.elementId()) {
        d->after = other->d->after;
        return true;
    } else {
        return false;
    }
}
