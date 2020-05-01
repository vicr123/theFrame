#ifndef ELEMENTSTATE_H
#define ELEMENTSTATE_H

#include <QJsonObject>
#include <QStack>

class Element;
struct ElementState
{
    ElementState(Element* element);
    ElementState();

    const QMetaObject* elementType;

    QStack<uint> parentIds;
    Element* rootElement;
    int index;

    uint elementId();
    Element* parentElement();

    QJsonObject data;
};

#endif // ELEMENTSTATE_H
