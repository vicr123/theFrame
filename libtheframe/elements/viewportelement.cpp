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
#include "viewportelement.h"

struct ViewportElementPrivate {
    QSize viewportSize = QSize(1920, 1080);
    QColor backgroundColor = Qt::white;
};

ViewportElement::ViewportElement() : Element() {
    d = new ViewportElementPrivate();

}

ViewportElement::~ViewportElement() {
    delete d;
}

void ViewportElement::setViewportSize(QSize size) {
    d->viewportSize = size;
    emit viewportSizeChanged(size);
}

QSize ViewportElement::viewportSize() {
    return d->viewportSize;
}

void ViewportElement::setBackgroundColor(QColor color) {
    d->backgroundColor = color;
}

QColor ViewportElement::backgroundColor() {
    return d->backgroundColor;
}

ViewportElement::FrameIterator ViewportElement::constFrameBegin() const {
    return FrameIterator(0);
}

ViewportElement::FrameIterator ViewportElement::constFrameEnd(quint64 numberOfFrames) const {
    return FrameIterator(numberOfFrames);
}

void ViewportElement::render(QPainter* painter, quint64 frame) const {
    painter->save();

    painter->setBrush(d->backgroundColor);
    painter->setPen(Qt::transparent);
    painter->drawRect(QRect(QPoint(0, 0), d->viewportSize));

    Element::render(painter, frame);

    painter->restore();
}

QMap<QString, Element::PropertyType> ViewportElement::staticProperties() const {
    return {};
}

QMap<QString, Element::PropertyType> ViewportElement::animatableProperties() const {
    return {};
}

QPoint ViewportElement::renderOffset(quint64 frame) const {
    Q_UNUSED(frame)
    return QPoint(0, 0);
}

QString ViewportElement::typeDisplayName() const {
    return tr("Canvas");
}

QString ViewportElement::propertyDisplayName(QString property) const {
    Q_UNUSED(property)
    return "";
}

QColor ViewportElement::propertyColor(QString property) const {
    Q_UNUSED(property)
    return QColor();
}

ViewportElement::FrameIterator::FrameIterator(quint64 value) {
    this->value = value;
}

void ViewportElement::FrameIterator::operator ++() {
    this->value++;
}

void ViewportElement::FrameIterator::operator--() {
    this->value--;
}

quint64 ViewportElement::FrameIterator::operator->() {
    return value;
}
