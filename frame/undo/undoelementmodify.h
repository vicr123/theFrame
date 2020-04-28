#ifndef UNDOELEMENTMODIFY_H
#define UNDOELEMENTMODIFY_H

#include <QUndoCommand>
#include "elementstate.h"

struct UndoElementModifyPrivate;
class UndoElementModify : public QUndoCommand
{
    public:
        UndoElementModify(QString text, ElementState before, ElementState after);
        ~UndoElementModify();

    private:
        UndoElementModifyPrivate* d;

        // QUndoCommand interface
    public:
        void undo();
        void redo();
        int id() const;
        bool mergeWith(const QUndoCommand* o);
};

#endif // UNDOELEMENTMODIFY_H
