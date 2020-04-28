#ifndef UNDOTIMELINEELEMENTMODIFY_H
#define UNDOTIMELINEELEMENTMODIFY_H

#include <QUndoCommand>
#include "timelineelementstate.h"

struct UndoTimelineElementModifyPrivate;
class UndoTimelineElementModify : public QUndoCommand
{
    public:
        UndoTimelineElementModify(QString text, TimelineElementState before, TimelineElementState after);
        ~UndoTimelineElementModify();

    private:
        UndoTimelineElementModifyPrivate* d;

        // QUndoCommand interface
    public:
        void undo();
        void redo();
        int id() const;
        bool mergeWith(const QUndoCommand* o);
};

#endif // UNDOTIMELINEELEMENTMODIFY_H
