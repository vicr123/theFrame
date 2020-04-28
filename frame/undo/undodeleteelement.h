#ifndef UNDODELETEELEMENT_H
#define UNDODELETEELEMENT_H

#include <QUndoCommand>
#include "elementstate.h"

struct UndoDeleteElementPrivate;
class UndoDeleteElement : public QUndoCommand
{
    public:
        UndoDeleteElement(QString text, ElementState oldElement);
        ~UndoDeleteElement();

    private:
        UndoDeleteElementPrivate* d;

        // QUndoCommand interface
    public:
        void undo();
        void redo();
};

#endif // UNDODELETEELEMENT_H
