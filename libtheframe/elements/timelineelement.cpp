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
#include "timelineelement.h"

#include <QTimer>
#include <QStack>

struct TimelineElementPrivate {
    quint64 startFrame;
    quint64 endFrame;
    QVariant startValue;
    QVariant endValue;

    QEasingCurve easingCurve;

    Element* parent;
    QString propertyName;

    QStack<TimelineElementPrivate> transactionStack;
};

TimelineElement::TimelineElement() : QObject(nullptr) {
    d = new TimelineElementPrivate();
}

TimelineElement::TimelineElement(quint64 startFrame, QVariant startValue, quint64 endFrame, QVariant endValue, QEasingCurve easingCurve) : QObject(nullptr) {
    d = new TimelineElementPrivate();
    d->startFrame = startFrame;
    d->startValue = startValue;
    d->endFrame = endFrame;
    d->endValue = endValue;
    d->easingCurve = easingCurve;
}

TimelineElement::~TimelineElement() {
    emit aboutToDelete();
    delete d;
}

void TimelineElement::setStartFrame(quint64 startFrame) {
    d->startFrame = startFrame;
    emit elementPropertyChanged();
}

quint64 TimelineElement::startFrame() {
    return d->startFrame;
}

void TimelineElement::setEndFrame(quint64 endFrame) {
    d->endFrame = endFrame;
    emit elementPropertyChanged();
}

quint64 TimelineElement::endFrame() {
    return d->endFrame;
}

void TimelineElement::setStartValue(QVariant startValue) {
    d->startValue = startValue;
    emit elementPropertyChanged();
}

QVariant TimelineElement::startValue() {
    return d->startValue;
}

void TimelineElement::setEndValue(QVariant endValue) {
    d->endValue = endValue;
    emit elementPropertyChanged();
}

QVariant TimelineElement::endValue() {
    return d->endValue;
}

void TimelineElement::setEasingCurve(QEasingCurve easingCurve) {
    d->easingCurve = easingCurve;
    emit elementPropertyChanged();
}

QEasingCurve TimelineElement::easingCurve() {
    return d->easingCurve;
}

QString TimelineElement::propertyName() {
    return d->propertyName;
}

Element* TimelineElement::parentElement() {
    return d->parent;
}

bool TimelineElement::isFrameContained(quint64 frame) {
    return d->startFrame <= frame && d->endFrame >= frame;
}

bool TimelineElement::intersects(TimelineElement* other) {
    return this->isFrameContained(other->endFrame()) || this->isFrameContained(other->startFrame());
}

void TimelineElement::beginTransaction() {
    TimelineElementPrivate currentState(*d);
    d->transactionStack.push(currentState);
}

void TimelineElement::commitTransaction() {
    if (d->transactionStack.length() > 0) d->transactionStack.pop();
}

void TimelineElement::rollbackTransaction() {
    if (d->transactionStack.length() > 0) {
        TimelineElementPrivate oldState = d->transactionStack.pop();
        *d = oldState;
    } else {
        this->deleteLater();
    }
}

void TimelineElement::setParentElement(Element* element) {
    d->parent = element;
}

void TimelineElement::setPropertyName(QString property) {
    d->propertyName = property;
}
