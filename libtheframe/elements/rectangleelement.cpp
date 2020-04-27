/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
 *   Copyright (C) 2020 Victor Tran
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * *************************************/
#include "rectangleelement.h"

struct RectangleElementPrivate {

};

RectangleElement::RectangleElement() : Element() {
    d = new RectangleElementPrivate();

    this->setStartValue("geometry", QRect(0, 0, 100, 100));
    this->setStartValue("backgroundColor", QColor(0, 100, 200));
    this->setStartValue("borderColor", QColor(0, 0, 0));
    this->setStartValue("opacity", 1.0);
}

RectangleElement::~RectangleElement() {
    delete d;
}

void RectangleElement::render(QPainter* painter, quint64 frame) const {
    painter->save();

    QRect geometry = this->propertyValueForFrame("geometry", frame).toRect();
    QColor backgroundColor = this->propertyValueForFrame("backgroundColor", frame).value<QColor>();
    QColor borderColor = this->propertyValueForFrame("borderColor", frame).value<QColor>();
    double opacity = this->propertyValueForFrame("opacity", frame).toDouble() * painter->opacity();

    geometry.translate(this->parentElement()->renderOffset(frame));

    painter->setOpacity(opacity);
    painter->setBrush(backgroundColor);
    painter->setPen(borderColor);
    painter->drawRect(geometry);

    Element::render(painter, frame);

    painter->restore();
}

QMap<QString, Element::PropertyType> RectangleElement::staticProperties() const {
    return {};
}

QMap<QString, Element::PropertyType> RectangleElement::animatableProperties() const {
    return {
        {QStringLiteral("geometry"), Rect},
        {QStringLiteral("backgroundColor"), Color},
        {QStringLiteral("borderColor"), Color},
        {QStringLiteral("opacity"), Percentage}
    };
}

QPoint RectangleElement::renderOffset(quint64 frame) const {
    QRect geometry = this->propertyValueForFrame("geometry", frame).toRect();
    return geometry.topLeft() + this->parentElement()->renderOffset(frame);
}

QString RectangleElement::typeDisplayName() const {
    return tr("Rectangle");
}

QString RectangleElement::propertyDisplayName(QString property) const {
    if (property == "geometry") {
        return tr("Geometry");
    } else if (property == "backgroundColor") {
        return tr("Background Color");
    } else if (property == "borderColor") {
        return tr("Border Color");
    } else if (property == "opacity") {
        return tr("Opacity");
    }
    return "";
}

QColor RectangleElement::propertyColor(QString property) const {
    if (property == "geometry") {
        return QColor(14, 40, 37, 127);
    } else if (property == "backgroundColor") {
        return QColor(38, 40, 21, 127);
    } else if (property == "borderColor") {
        return QColor(40, 18, 35, 127);
    } else if (property == "opacity") {
        return QColor(39, 40, 23, 127);
    }
    return QColor();
}

