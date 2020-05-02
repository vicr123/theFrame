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
#ifndef TIMELINEELEMENT_H
#define TIMELINEELEMENT_H

#include <QObject>
#include <QVariant>
#include <QEasingCurve>

#include "libtheframe_global.h"

struct TimelineElementPrivate;
class Element;
class LIBTHEFRAME_EXPORT TimelineElement : public QObject {
        Q_OBJECT
    public:
        explicit TimelineElement(Element* parentElement = nullptr);
        explicit TimelineElement(quint64 startFrame, QVariant startValue, quint64 endFrame, QVariant endValue, QEasingCurve easingCurve = QEasingCurve::Linear);
        ~TimelineElement();

        void setStartFrame(quint64 startFrame);
        void moveStartFrame(quint64 startFrame);
        quint64 startFrame();

        void setEndFrame(quint64 endFrame);
        quint64 endFrame();

        quint64 length();

        void setStartValue(QVariant startValue);
        QVariant startValue();

        void setStartAnchored(bool anchor);
        bool startAnchored();

        void setEndValue(QVariant endValue);
        QVariant endValue();

        void setEasingCurve(QEasingCurve easingCurve);
        QEasingCurve easingCurve();

        QString propertyName();
        Element* parentElement();

        uint getId();

        bool isFrameContained(quint64 frame);
        bool intersects(TimelineElement* other);

        QJsonObject save();
        void load(QJsonObject obj);

    protected:
        friend Element;
        void beginTransaction();
        void commitTransaction();
        void rollbackTransaction();
        void setParentElement(Element* element);
        void setPropertyName(QString property);
        void setId(uint id);

    signals:
        void elementPropertyChanged();
        void aboutToDelete();

    private:
        TimelineElementPrivate* d;
};

#endif // TIMELINEELEMENT_H
