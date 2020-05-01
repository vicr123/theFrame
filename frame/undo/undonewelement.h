#ifndef UNDONEWELEMENT_H
#define UNDONEWELEMENT_H

#include <QUndoCommand>
#include "elementstate.h"

struct UndoNewElementPrivate;
class UndoNewElement : public QUndoCommand
{
    public:
        UndoNewElement(QString text, ElementState newElement);
        UndoNewElement(QString text, QList<ElementState> newElementStates);
        ~UndoNewElement();

    private:
        UndoNewElementPrivate* d;

        // QUndoCommand interface
    public:
        void undo();
        void redo();
};

#endif // UNDONEWELEMENT_H
