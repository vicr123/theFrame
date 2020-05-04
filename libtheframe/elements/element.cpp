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
#include "element.h"

#include "timelineelement.h"
#include <tvariantanimation.h>
#include <QRandomGenerator>
#include <QJsonObject>
#include <QJsonArray>

#include "viewportelement.h"
#include "rectangleelement.h"
#include "textelement.h"
#include "groupelement.h"
#include "pictureelement.h"
#include "ellipseelement.h"

struct ElementPrivate {
    QMultiMap<QString, TimelineElement*> timelineElements;
    QMap<uint, TimelineElement*> timelineElementsById;
    QMap<QString, QVariant> startValues;

    QString elementName;
    QColor displayColor;

    QList<Element*> children;
    QMap<uint, Element*> childrenById;
    Element* parent = nullptr;

    const QList<QColor> defaultColors = {
        QColor(0, 40, 40, 127),
        QColor(40, 0, 40, 127),
        QColor(40, 40, 0, 127),
        QColor(40, 0, 0, 127),
        QColor(0, 40, 0, 127),
        QColor(0, 0, 40, 127)
    };

    int inTransaction = 0;
    bool invalidateFrames = false;
    quint64 invalidateFrom;

    uint timelineElementIds = 1;
    uint childrenElementIds = 1;
    uint thisId = 0;
};

Element::Element() : QObject(nullptr) {
    d = new ElementPrivate();
    d->elementName = tr("New Element");

    int colorIndex = QRandomGenerator::system()->bounded(d->defaultColors.count());
    d->displayColor = d->defaultColors.at(colorIndex);
}

Element::~Element() {
    for (Element* element : d->children) {
        element->deleteLater();
    }

    delete d;
}

void Element::render(QPainter* painter, quint64 frame) const {
    for (Element* element : d->children) {
        element->render(painter, frame);
    }
}

QMap<QString, Element::PropertyType> Element::allProperties() const {
    QMap<QString, PropertyType> map;
    map.unite(this->animatableProperties());
    map.unite(this->staticProperties());
    return map;
}

QVariantMap Element::propertyMetadata(QString property) const
{
    return QVariantMap();
}

Element::PropertyType Element::propertyType(QString property) const {
    Q_ASSERT(this->allProperties().contains(property));
    return this->allProperties().value(property);
}

QString Element::name() const {
    return d->elementName;
}

void Element::setName(QString name) {
    d->elementName = name;
    emit nameChanged(name);
}

void Element::setStartValue(QString property, QVariant value) {
    Q_ASSERT(this->allProperties().contains(property));
    d->startValues.insert(property, value);
    tryInvalidateFromFrame(0);
}

QVariant Element::startValue(QString property) const {
    Q_ASSERT(this->allProperties().contains(property));
    return d->startValues.value(property);
}

QList<TimelineElement*> Element::timelineElements(QString property) const {
    Q_ASSERT(this->animatableProperties().contains(property));
    return d->timelineElements.values(property);
}

TimelineElement* Element::timelineElementById(uint id) const
{
    return d->timelineElementsById.value(id);
}

void Element::addTimelineElement(QString property, TimelineElement* element, uint id) {
    Q_ASSERT(this->animatableProperties().contains(property));

    uint timelineElementId;
    if (id != 0) {
        timelineElementId = id;
    } else {
        timelineElementId = d->timelineElementIds;
    }

    element->setParentElement(this);
    element->setPropertyName(property);
    element->setId(timelineElementId);
    d->timelineElements.insert(property, element);
    d->timelineElementsById.insert(timelineElementId, element);
    connect(element, &TimelineElement::elementPropertyChanged, this, [ = ] {
        emit timelineElementsChanged();
        tryInvalidateFromFrame(element->startFrame());
    });
    connect(element, &TimelineElement::aboutToDelete, this, [ = ] {
        tryInvalidateFromFrame(element->startFrame());
        d->timelineElements.remove(property, element);
        if (d->timelineElementsById.value(timelineElementId) == element) d->timelineElementsById.remove(timelineElementId);
        emit timelineElementsChanged();
    });

    d->timelineElementIds++;

    tryInvalidateFromFrame(element->startFrame());
    emit timelineElementsChanged();
}

void Element::clearTimelineElements()
{
    for (TimelineElement* element : d->timelineElements.values()) {
        element->deleteLater();
    }
    d->timelineElements.clear();
    d->timelineElementsById.clear();
    d->timelineElementIds = 1;
}

QVariant Element::propertyValueForFrame(QString property, quint64 frame) const {
    Q_ASSERT(this->animatableProperties().contains(property));

    TimelineElement* closestTimelineElement = nullptr;

    for (TimelineElement* timelineElement : d->timelineElements.values(property)) {
        if (timelineElement->endFrame() <= frame && (!closestTimelineElement || timelineElement->startFrame() > closestTimelineElement->endFrame())) {
            closestTimelineElement = timelineElement;
        } else if (timelineElement->isFrameContained(frame)) {
            //This property is currently animating within this timeline element
            closestTimelineElement = timelineElement;
            break;
        }
    }

    if (closestTimelineElement) {
        if (closestTimelineElement->endFrame() < frame) {
            return closestTimelineElement->endValue();
        } else {
            tVariantAnimation anim;
            if (closestTimelineElement->startAnchored()) {
                if (closestTimelineElement->startFrame() == 0) {
                    anim.setStartValue(this->startValue(property));
                } else {
                    anim.setStartValue(propertyValueForFrame(property, closestTimelineElement->startFrame() - 1));
                }
            } else {
                anim.setStartValue(closestTimelineElement->startValue());
            }
            anim.setEndValue(closestTimelineElement->endValue());
            anim.setDuration(static_cast<int>(closestTimelineElement->endFrame() - closestTimelineElement->startFrame()));
            anim.setEasingCurve(closestTimelineElement->easingCurve());
            anim.setCurrentTime(static_cast<int>(frame - closestTimelineElement->startFrame()));
            anim.setForceAnimation(true);
            return anim.currentValue();
        }
    }

    return startValue(property);
}

void Element::addChild(Element* element, uint id) {
    this->insertChild(d->children.count(), element, id);
}

void Element::insertChild(int index, Element* element, uint id)
{

    uint childElementId;
    if (id != 0) {
        childElementId = id;
    } else {
        childElementId = d->childrenElementIds;
    }

    d->children.insert(index, element);
    d->childrenById.insert(childElementId, element);
    element->d->parent = this;
    element->d->thisId = childElementId;
    connect(element, &Element::destroyed, this, [ = ] {
        d->children.removeOne(element);
        if (d->childrenById.value(childElementId) == element) d->childrenById.remove(childElementId);
        tryInvalidateFromFrame(0);
    });

    d->childrenElementIds++;

    emit childElementInserted(index, element);
    tryInvalidateFromFrame(0);
}

QList<Element*> Element::childElements() const {
    return d->children;
}

void Element::clearChildren() {
    for (Element* element : d->children) {
        element->deleteLater();
    }
    d->childrenById.clear();
    d->children.clear();
    d->childrenElementIds = 1;
}

Element* Element::parentElement() const {
    return d->parent;
}

Element* Element::childById(uint id) const
{
    return d->childrenById.value(id);
}

const Element* Element::rootElement() const {
    const Element* e = this;
    while (e->parentElement()) {
        e = e->parentElement();
    }
    return e;
}

TimelineElement* Element::timelineElementAtFrame(QString property, quint64 frame) const {
    if (property == "") return nullptr;
    for (TimelineElement* timelineElement : timelineElements(property)) {
        if (timelineElement->isFrameContained(frame)) {
            return timelineElement;
        }
    }
    return nullptr;
}

void Element::beginTransaction() {
    d->inTransaction++;
    for (TimelineElement* element : d->timelineElements) {
        element->beginTransaction();
    }
    qDebug() << "Transaction +" << d->elementName;

}

bool Element::tryCommitTransaction() {
    if (!this->canCommitTransaction()) {
        rollbackTransaction();
        return false;
    }

    for (TimelineElement* element : d->timelineElements) {
        element->commitTransaction();
    }

    d->inTransaction--;
    qDebug() << "Transaction -" << d->elementName;

    if (d->invalidateFrames) {
        this->tryInvalidateFromFrame(d->invalidateFrom);
        d->invalidateFrames = false;
    }

    emit timelineElementsChanged();
    return true;
}

bool Element::canCommitTransaction()
{
    //Make sure the state of the timeline elements are valid
    for (QString property : this->animatableProperties().keys()) {
        for (TimelineElement* element1 : d->timelineElements.values(property)) {
            //Make sure elements don't intersect each other
            for (TimelineElement* element2 : d->timelineElements.values(property)) {
                if (element1 != element2 && element1->intersects(element2)) return false;
            }

            //Make sure elements are sequential
            if (element1->startFrame() > element1->endFrame()) return false;
        }
    }
    return true;
}

void Element::rollbackTransaction()
{
    for (TimelineElement* element : d->timelineElements) {
        element->rollbackTransaction();
    }

    d->inTransaction--;
    qDebug() << "Transaction -" << d->elementName;
    emit timelineElementsChanged();
}

void Element::setDisplayColor(QColor color) {
    color.setAlpha(127);
    d->displayColor = color;
    emit displayColorChanged(color);
}

QColor Element::displayColor() const {
    return d->displayColor;
}

QJsonObject Element::save() const {
    QJsonObject elementObject;
    elementObject.insert("type", QString::fromLatin1(this->metaObject()->className()));
    elementObject.insert("name", this->name());
    elementObject.insert("displayColor", propertyToJson(Color, this->displayColor()));
    elementObject.insert("id", QString::number(d->thisId));
    elementObject.insert("timelineElementIds", QString::number(d->timelineElementIds));
    elementObject.insert("childrenElementIds", QString::number(d->childrenElementIds));

    QJsonObject startValues;
    for (QString property : this->allProperties().keys()) {
        startValues.insert(property, propertyToJson(property, this->startValue(property)));
    }
    elementObject.insert("startValues", startValues);

    QJsonArray timelineElements;
    for (TimelineElement* element : d->timelineElements) {
        timelineElements.append(element->save());
    }
    elementObject.insert("timelineElements", timelineElements);

    QJsonArray children;
    for (Element* child : d->children) {
        children.append(child->save());
    }
    elementObject.insert("children", children);

    return elementObject;
}

bool Element::load(QJsonObject obj, bool respectIds) {
    this->clearTimelineElements();
    this->clearChildren();

    //Load the ID here so that the root object doesn't need to be set an ID
    if (respectIds) d->thisId = obj.value("id").toString().toUInt();
    d->timelineElementIds = obj.value("timelineElementIds").toString().toUInt();
    d->childrenElementIds = obj.value("childrenElementIds").toString().toUInt();
    if (obj.value("type").toString() != this->metaObject()->className()) return false;
    this->setName(obj.value("name").toString());
    this->setDisplayColor(jsonToProperty(Color, obj.value("displayColor")).value<QColor>());

    QJsonObject startValues = obj.value("startValues").toObject();
    for (QString property : this->allProperties().keys()) {
        this->setStartValue(property, jsonToProperty(property, startValues.value(property)));
    }

    QJsonArray timelineElements = obj.value("timelineElements").toArray();
    for (QJsonValue elementValue : timelineElements) {
        QJsonObject elementObject = elementValue.toObject();
        QString property = elementObject.value("property").toString();

        TimelineElement* element = new TimelineElement(this);
        element->load(elementObject);

        uint id = 0;
        if (respectIds) id = elementObject.value("id").toString().toUInt();
        this->addTimelineElement(property, element, id);
    }

    QJsonArray children = obj.value("children").toArray();
    for (QJsonValue child : children) {
        QJsonObject childObject = child.toObject();
        Element* childElement = constructByType(childObject.value("type").toString());

        if (childElement) {
            //Also load the ID here so that this element knows about it
            uint id = 0;
            if (respectIds) id = childObject.value("id").toString().toUInt();
            childElement->load(childObject);
            this->addChild(childElement, id);
        }
    }

    return true;
}

uint Element::getId()
{
    return d->thisId;
}

Element* Element::constructByType(QString type)
{
    if (type == "RectangleElement") {
        return new RectangleElement();
    } else if (type == "ViewportElement") {
        return new ViewportElement();
    } else if (type == "TextElement") {
        return new TextElement();
    } else if (type == "GroupElement") {
        return new GroupElement();
    } else if (type == "PictureElement") {
        return new PictureElement();
    } else if (type == "EllipseElement") {
        return new EllipseElement();
    }
    return nullptr;
}

QJsonValue Element::propertyToJson(QString property, QVariant value) const {
    return propertyToJson(propertyType(property), value);
}

QJsonValue Element::propertyToJson(Element::PropertyType propertyType, QVariant value) const {
    switch (propertyType) {
        case Element::Integer:
        case Element::Double:
        case Element::Percentage:
        case Element::String:
        case Element::File:
            return QJsonValue::fromVariant(value);
        case Element::Rect: {
            QRect rect = value.toRect();
            QJsonArray array = {
                rect.x(),
                rect.y(),
                rect.width(),
                rect.height()
            };
            return array;
        }
        case Element::Color: {
            QColor col = value.value<QColor>();
            QJsonArray array = {
                col.red(),
                col.green(),
                col.blue()
            };
            return array;
        }
        case Element::Point: {
            QPoint point = value.toPoint();
            QJsonArray array = {
                point.x(),
                point.y()
            };
            return array;
        }
        case Element::Font: {
            QFont font = value.value<QFont>();
            return font.toString();
        }
    }
    return QJsonValue();
}

QVariant Element::jsonToProperty(QString property, QJsonValue json) const {
    return jsonToProperty(propertyType(property), json);
}

QVariant Element::jsonToProperty(Element::PropertyType propertyType, QJsonValue json) const {
    switch (propertyType) {
        case Element::Integer:
        case Element::Double:
        case Element::Percentage:
        case Element::String:
        case Element::File:
            return json.toVariant();
        case Element::Rect: {
            QJsonArray array = json.toArray();
            QRect rect(array.at(0).toInt(), array.at(1).toInt(), array.at(2).toInt(), array.at(3).toInt());
            return rect;
        }
        case Element::Color: {
            QJsonArray array = json.toArray();
            QColor col(array.at(0).toInt(), array.at(1).toInt(), array.at(2).toInt());
            return col;
        }
        case Element::Point: {
            QJsonArray array = json.toArray();
            QPoint point(array.at(0).toInt(), array.at(1).toInt());
            return point;
        }
        case Element::Font: {
            QString str = json.toString();
            QFont font;
            font.fromString(str);
            return font;
        }
    }
    return QVariant();
}

void Element::tryInvalidateFromFrame(quint64 frame) {
    if (d->inTransaction != 0) {
        if (d->invalidateFrames > frame || !d->invalidateFrames) d->invalidateFrom = frame;
        d->invalidateFrames = true;
    } else {
        Element* e = this;
        do {
            emit e->invalidateFromFrame(frame);
        } while ((e = e->parentElement()) != nullptr);
    }
}


