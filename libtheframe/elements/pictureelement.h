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
#ifndef PICTUREELEMENT_H
#define PICTUREELEMENT_H

#include "element.h"

#include "libtheframe_global.h"

struct PictureElementPrivate;
class LIBTHEFRAME_EXPORT PictureElement : public Element {
        Q_OBJECT
    public:
        explicit Q_INVOKABLE PictureElement();
        ~PictureElement();

        void setStartValue(QString property, QVariant value);

    signals:

    private:
        PictureElementPrivate* d;

        // Element interface
    public:
        void render(QPainter* painter, quint64 frame) const;
        QPoint renderOffset(quint64 frame) const;
        QMap<QString, PropertyType> animatableProperties() const;
        QMap<QString, PropertyType> staticProperties() const;
        QString propertyDisplayName(QString property) const;
        QColor propertyColor(QString property) const;
        QString typeDisplayName() const;
};

#endif // PICTUREELEMENT_H
