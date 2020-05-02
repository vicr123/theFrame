#ifndef TIMELINEELEMENTSTATE_H
#define TIMELINEELEMENTSTATE_H

#include <QVariant>
#include <QEasingCurve>
#include <QStack>
#include <QJsonObject>

class TimelineElement;
class Element;
struct TimelineElementState
{
    TimelineElementState(TimelineElement* element);
    TimelineElementState();

    Element* target();

    QJsonObject data;
    QString elementProperty();
    uint elementId();

    Element* rootElement;
    QStack<uint> elementIds;

    bool operator==(TimelineElementState other);
    bool operator!=(TimelineElementState other);
};

#endif // TIMELINEELEMENTSTATE_H
