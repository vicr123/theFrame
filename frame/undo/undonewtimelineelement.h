#ifndef UNDONEWTIMELINEELEMENT_H
#define UNDONEWTIMELINEELEMENT_H

#include <QUndoCommand>
#include "timelineelementstate.h"

struct UndoNewTimelineElementPrivate;
class UndoNewTimelineElement : public QUndoCommand
{
    public:
        UndoNewTimelineElement(QString text, TimelineElementState newTimelineElement);
        ~UndoNewTimelineElement();

    private:
        UndoNewTimelineElementPrivate* d;

        // QUndoCommand interface
    public:
        void undo();
        void redo();
};

#endif // UNDONEWTIMELINEELEMENT_H
