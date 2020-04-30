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
#ifndef ELEMENT_H
#define ELEMENT_H

#include <QObject>
#include <QPainter>
#include <QVariant>

#include "libtheframe_global.h"

struct ElementPrivate;
class TimelineElement;
class LIBTHEFRAME_EXPORT Element : public QObject {
        Q_OBJECT
    public:
        explicit Element();
        ~Element();

        enum PropertyType {
            Integer,
            Double,
            Rect,
            Color,
            Percentage,
            String,
            Point,
            Font,
            File
        };

        virtual void render(QPainter* painter, quint64 frame) const;
        virtual QPoint renderOffset(quint64 frame) const = 0;

        QMap<QString, PropertyType> allProperties() const;
        virtual QMap<QString, PropertyType> animatableProperties() const = 0;
        virtual QMap<QString, PropertyType> staticProperties() const = 0;
        virtual QString propertyDisplayName(QString property) const = 0;
        virtual QColor propertyColor(QString property) const = 0;
        virtual QVariantMap propertyMetadata(QString property) const;
        PropertyType propertyType(QString property) const;

        virtual QString typeDisplayName() const = 0;
        void setName(QString name);
        QString name() const;

        virtual void setStartValue(QString property, QVariant value);
        QVariant startValue(QString property) const;

        QList<TimelineElement*> timelineElements(QString property) const;
        TimelineElement* timelineElementById(uint id) const;
        void addTimelineElement(QString property, TimelineElement* element, uint id = 0);
        void clearTimelineElements();

        QVariant propertyValueForFrame(QString property, quint64 frame) const;

        void addChild(Element* element, uint id = 0);
        QList<Element*> childElements() const;
        void clearChildren();
        Element* parentElement() const;
        Element* childById(uint id) const;
        const Element* rootElement() const;

        TimelineElement* timelineElementAtFrame(QString property, quint64 frame) const;

        void beginTransaction();
        bool tryCommitTransaction();

        void setDisplayColor(QColor color);
        QColor displayColor() const;

        QJsonObject save() const;
        bool load(QJsonObject obj);

        uint getId();

    protected:

    signals:
        void timelineElementsChanged();
        void invalidateFromFrame(quint64 frame);
        void nameChanged(QString name);
        void newChildElement(Element* element);
        void displayColorChanged(QColor color);

    private:
        ElementPrivate* d;

        QJsonValue propertyToJson(QString property, QVariant value) const;
        QJsonValue propertyToJson(PropertyType propertyType, QVariant value) const;
        QVariant jsonToProperty(QString property, QJsonValue json) const;
        QVariant jsonToProperty(PropertyType propertyType, QJsonValue json) const;
        void tryInvalidateFromFrame(quint64 frame);
};

#endif // ELEMENT_H
