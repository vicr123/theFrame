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
#include "groupelement.h"

GroupElement::GroupElement() : Element() {
    this->setStartValue("opacity", 1.0);
}


void GroupElement::render(QPainter* painter, quint64 frame) const {
    painter->save();

    double opacity = this->propertyValueForFrame("opacity", frame).toDouble();

    painter->setOpacity(painter->opacity() * opacity);
    Element::render(painter, frame);

    painter->restore();
}

QPoint GroupElement::renderOffset(quint64 frame) const {
    QPoint position = this->propertyValueForFrame("offset", frame).toPoint();
    return position + this->parentElement()->renderOffset(frame);
}

QMap<QString, Element::PropertyType> GroupElement::animatableProperties() const {
    return {
        {QStringLiteral("offset"), Element::Point},
        {QStringLiteral("opacity"), Element::Percentage}
    };
}

QMap<QString, Element::PropertyType> GroupElement::staticProperties() const {
    return {};
}

QString GroupElement::propertyDisplayName(QString property) const {
    if (property == "offset") {
        return tr("Offset");
    } else if (property == "opacity") {
        return tr("Opacity");
    }
    return QString();
}

QColor GroupElement::propertyColor(QString property) const {
    if (property == "text") {
        return QColor(14, 40, 37, 127);
    } else if (property == "opacity") {
        return QColor(38, 40, 21, 127);
    }
    return QColor();
}

QString GroupElement::typeDisplayName() const {
    return tr("Group");
}
