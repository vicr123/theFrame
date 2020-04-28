#ifndef UNDODELETETIMELINEELEMENT_H
#define UNDODELETETIMELINEELEMENT_H

#include <QUndoCommand>
#include "timelineelementstate.h"

struct UndoDeleteTimelineElementPrivate;
class UndoDeleteTimelineElement : public QUndoCommand
{
    public:
        UndoDeleteTimelineElement(QString text, TimelineElementState oldTimelineElement);
        ~UndoDeleteTimelineElement();

    private:
        UndoDeleteTimelineElementPrivate* d;

        // QUndoCommand interface
    public:
        void undo();
        void redo();
};

#endif // UNDODELETETIMELINEELEMENT_H
