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
#ifndef VIEWPORTELEMENT_H
#define VIEWPORTELEMENT_H

#include "element.h"

#include "libtheframe_global.h"

struct ViewportElementPrivate;
class LIBTHEFRAME_EXPORT ViewportElement : public Element {
        Q_OBJECT
    public:
        struct FrameIterator {
            FrameIterator(quint64 value);
            quint64 value;

            void operator++();
            void operator--();
            quint64 operator->();
        };

        explicit Q_INVOKABLE ViewportElement();
        ~ViewportElement();

        void setViewportSize(QSize size);
        QSize viewportSize();

        void setBackgroundColor(QColor color);
        QColor backgroundColor();

        FrameIterator constFrameBegin() const;
        FrameIterator constFrameEnd(quint64 numberOfFrames) const;

    signals:
        void viewportSizeChanged(QSize size);

    private:
        ViewportElementPrivate* d;

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

#endif // VIEWPORTELEMENT_H
