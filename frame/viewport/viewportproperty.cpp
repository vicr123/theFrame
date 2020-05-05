#include "viewportproperty.h"

#include "viewport.h"

#include "rectangleviewport.h"
#include "pointviewport.h"

struct ViewportPropertyPrivate {
    ViewportProperty::Type type;
    Viewport* parent;
};

ViewportProperty::ViewportProperty(ViewportProperty::Type type, Viewport* parent) : QWidget(parent)
{
    d = new ViewportPropertyPrivate();
    d->type = type;
    d->parent = parent;
}

ViewportProperty::~ViewportProperty()
{
    delete d;
}

ViewportProperty* ViewportProperty::constructForType(Element::PropertyType propertyType, Type type, Viewport* viewport)
{
    switch (propertyType) {
        case Element::Rect:
            return new RectangleViewport(type, viewport);
        case Element::Point:
            return new PointViewport(type, viewport);
        case Element::Integer:
        case Element::Double:
        case Element::Color:
        case Element::Percentage:
        case Element::String:
        case Element::Font:
        case Element::File:
            break;
    }
    return nullptr;
}
