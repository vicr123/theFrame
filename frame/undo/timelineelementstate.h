#ifndef TIMELINEELEMENTSTATE_H
#define TIMELINEELEMENTSTATE_H

#include <QVariant>
#include <QEasingCurve>
#include <QStack>

class TimelineElement;
class Element;
struct TimelineElementState
{
    TimelineElementState(TimelineElement* element);
    TimelineElementState();

    Element* target();

    QString property;
    quint64 startFrame;
    quint64 endFrame;
    QVariant startValue;
    QVariant endValue;
    QEasingCurve easingCurve;
    bool anchorStart;
    uint id;

    Element* rootElement;
    QStack<uint> elementIds;

    bool operator==(TimelineElementState other);
    bool operator!=(TimelineElementState other);
};

#endif // TIMELINEELEMENTSTATE_H
