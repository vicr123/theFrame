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
#ifndef RECTANGLEELEMENT_H
#define RECTANGLEELEMENT_H

#include "element.h"

#include "libtheframe_global.h"

struct RectangleElementPrivate;
class LIBTHEFRAME_EXPORT RectangleElement : public Element {
        Q_OBJECT
    public:
        explicit RectangleElement();
        ~RectangleElement();

    signals:

    private:
        RectangleElementPrivate* d;

        // Element interface
    public:
        void render(QPainter* painter, quint64 frame) const;
        QMap<QString, PropertyType> animatableProperties() const;
        QMap<QString, PropertyType> staticProperties() const;
        QPoint renderOffset(quint64 frame) const;
        QString typeDisplayName() const;
        QString propertyDisplayName(QString property) const;
        QColor propertyColor(QString property) const;
};

#endif // RECTANGLEELEMENT_H
