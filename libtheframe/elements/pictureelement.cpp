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
#include "pictureelement.h"

#include <QDir>
#include <QSvgRenderer>

struct PictureElementPrivate {
    bool reloadRequired = false;
};

PictureElement::PictureElement() : Element() {
    d = new PictureElementPrivate();

    this->setStartValue("source", "");
    this->setStartValue("opacity", 1.0);
    this->setStartValue("geometry", QRect(0, 0, 100, 100));
}

PictureElement::~PictureElement() {
    delete d;
}

void PictureElement::setStartValue(QString property, QVariant value) {
    Element::setStartValue(property, value);

    if (property == "source") d->reloadRequired = true;
}


void PictureElement::render(QPainter* painter, quint64 frame) const {
    QString file = QDir(this->rootElement()->property("projectPath").toString()).absoluteFilePath(this->startValue("source").toString());
    painter->save();

    QRect geometry = this->propertyValueForFrame("geometry", frame).toRect();
    double opacity = this->propertyValueForFrame("opacity", frame).toDouble() * painter->opacity();

    geometry.translate(this->parentElement()->renderOffset(frame));

    painter->setOpacity(opacity);

    QSvgRenderer renderer;
    if (renderer.load(file)) {
        renderer.render(painter, geometry);
    } else {
        QImage sourceImage;
        sourceImage.load(file);
        painter->drawImage(geometry, sourceImage);
    }

    Element::render(painter, frame);

    painter->restore();
}

QPoint PictureElement::renderOffset(quint64 frame) const {
    QRect geometry = this->propertyValueForFrame("geometry", frame).toRect();
    return geometry.topLeft() + this->parentElement()->renderOffset(frame);
}

QMap<QString, Element::PropertyType> PictureElement::animatableProperties() const {
    return {
        {"geometry", Element::Rect},
        {"opacity", Element::Percentage}
    };
}

QMap<QString, Element::PropertyType> PictureElement::staticProperties() const {
    return {
        {"source", Element::File}
    };
}

QString PictureElement::propertyDisplayName(QString property) const {
    if (property == "geometry") {
        return tr("Geometry");
    } else if (property == "source") {
        return tr("Source");
    } else if (property == "opacity") {
        return tr("Opacity");
    }
    return QString();
}

QColor PictureElement::propertyColor(QString property) const {
    if (property == "geometry") {
        return QColor(14, 40, 37, 127);
    } else if (property == "source") {
        return QColor(38, 40, 21, 127);
    } else if (property == "opacity") {
        return QColor(40, 18, 35, 127);
    }
    return QColor();
}

QString PictureElement::typeDisplayName() const {
    return tr("Picture");
}
