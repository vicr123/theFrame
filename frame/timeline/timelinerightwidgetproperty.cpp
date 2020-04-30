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
#include "timelinerightwidgetproperty.h"

#include <QPointer>
#include <QPainter>
#include <QMouseEvent>
#include <elements/timelineelement.h>
#include "timeline.h"
#include "prerenderer.h"

#include "undo/undotimelineelementmodify.h"
#include "undo/undonewtimelineelement.h"

struct TimelineRightWidgetPropertyPrivate {
    enum MouseState {
        MouseIdle,
        MouseDownNoAction,
        MouseMidTransition,
        MouseLeadTransition,
        MouseTailTransition,
        MouseNotOnTransition,
        MouseCreatingTransition
    };

    Timeline* timeline;
    QPointer<Element> element;
    QString property;

    bool isRoot;

    MouseState mouseState = MouseIdle;
    quint64 mouseFrameStart;
    quint64 initialElementFrameStart;
    QPointer<TimelineElement> mouseTimelineElement;
    TimelineElementState oldState;
};

TimelineRightWidgetProperty::TimelineRightWidgetProperty(Timeline* timeline, Element* element, QString property, bool isRoot, QWidget* parent) : QWidget(parent) {
    d = new TimelineRightWidgetPropertyPrivate();
    d->timeline = timeline;
    d->element = element;
    d->property = property;
    d->isRoot = isRoot;

    connect(timeline, &Timeline::frameSpacingChanged, this, &TimelineRightWidgetProperty::timerParametersChanged);
    connect(timeline, &Timeline::frameCountChanged, this, &TimelineRightWidgetProperty::timerParametersChanged);
    connect(timeline, &Timeline::framerateChanged, this, &TimelineRightWidgetProperty::timerParametersChanged);
    connect(timeline, &Timeline::currentSelectionChanged, this, &TimelineRightWidgetProperty::draw);
    connect(timeline, &Timeline::currentFrameChanged, this, &TimelineRightWidgetProperty::draw);
    timerParametersChanged();

    if (isRoot) {
        connect(timeline->prerenderer(), &Prerenderer::framePrerenderStateChanged, this, &TimelineRightWidgetProperty::draw);
        connect(timeline->prerenderer(), &Prerenderer::prerenderInvalidated, this, &TimelineRightWidgetProperty::draw);
        connect(timeline, &Timeline::inOutPointChanged, this, &TimelineRightWidgetProperty::draw);
    }

    connect(element, &Element::timelineElementsChanged, this, &TimelineRightWidgetProperty::draw);
    connect(element, &Element::displayColorChanged, this, &TimelineRightWidgetProperty::draw);

    this->setFocusPolicy(Qt::StrongFocus);
    this->setMouseTracking(true);
}

TimelineRightWidgetProperty::~TimelineRightWidgetProperty() {
    delete d;
}

quint64 TimelineRightWidgetProperty::frameForPoint(int x) {
    if (x < 0) return 0;
    if (x > this->width()) return static_cast<quint64>(this->width() / d->timeline->frameSpacing());
    return static_cast<quint64>(x / d->timeline->frameSpacing());
}

void TimelineRightWidgetProperty::paintEvent(QPaintEvent* event) {
    if (!d->element) return;

    QPainter painter(this);
    painter.setPen(Qt::transparent);
    if (d->property == "") {
        QColor displayColor = d->element->displayColor();
        if (d->isRoot) displayColor.setAlpha(255);
        painter.setBrush(displayColor);
    } else {
        painter.setBrush(d->element->propertyColor(d->property));
    }
    painter.drawRect(0, 0, this->width(), this->height());

    //Draw background elements
    for (quint64 i = 0; i < d->timeline->frameCount(); i++) {
        painter.save();
        double frameStart = d->timeline->frameSpacing() * static_cast<double>(i);
        double frameEnd = d->timeline->frameSpacing() * static_cast<double>(i + 1);

        QRectF contentRect;
        contentRect.setWidth(frameEnd - frameStart);
        contentRect.setHeight(this->height());
        contentRect.moveLeft(frameStart);
        contentRect.moveTop(0);
        if (contentRect.width() < 1) contentRect.setWidth(1);

        if (d->isRoot) {
            if (d->timeline->prerenderer()->isFrameCached(i)) {
                painter.setPen(Qt::transparent);
                painter.setBrush(QColor(0, 100, 0, 255));
                painter.drawRect(contentRect);
            }

            if (d->timeline->isInPreviewRange(i)) {
                QRectF previewIndicator = contentRect;
                previewIndicator.setHeight(contentRect.height() / 4);

                painter.setPen(Qt::transparent);
                painter.setBrush(QColor(255, 100, 0, 255));
                painter.drawRect(previewIndicator);
            }
        }

        if (!d->property.isEmpty()) {
            TimelineElement* timelineElement = d->element->timelineElementAtFrame(d->property, i);
            if (timelineElement) {
                painter.setPen(Qt::transparent);
                if (d->timeline->currentSelection().contains(timelineElement)) {
                    painter.setBrush(QColor(255, 200, 0));
                } else {
                    painter.setBrush(QColor(255, 100, 0));
                }
                painter.drawRect(contentRect);
            }
        }
        painter.restore();
    }

    //Draw foreground elements
    int skipText = 0;    for (quint64 i = 0; i < d->timeline->frameCount(); i++) {
        painter.save();
        double frameStart = d->timeline->frameSpacing() * static_cast<double>(i);
        double frameEnd = d->timeline->frameSpacing() * static_cast<double>(i + 1);

        QRectF contentRect;
        contentRect.setWidth(frameEnd - frameStart);
        contentRect.setHeight(this->height());
        contentRect.moveLeft(frameStart);
        contentRect.moveTop(0);
        if (contentRect.width() < 1) contentRect.setWidth(1);

        bool drawLine = false;
        if (skipText == 0) {
            if (d->timeline->frameSpacing() > 4) drawLine = true;

            if (d->isRoot) {
                QString number;
                bool isImportant;
                if (i % d->timeline->framerate() == 0) {
                    number = QString::number(i / d->timeline->framerate());

                    QFont fnt = painter.font();
                    fnt.setBold(true);
                    painter.setFont(fnt);
                    isImportant = true;
                    drawLine = true;
                } else {
                    number = QString::number(i % d->timeline->framerate());
                    isImportant = false;
                }

                if (d->timeline->frameSpacing() > 4 || isImportant) {
                    QRectF textRect = contentRect;

                    QFontMetrics metrics = painter.fontMetrics();
                    if (metrics.horizontalAdvance(number) + 6 > contentRect.width()) {
                        skipText = static_cast<int>((metrics.horizontalAdvance(number) + 6) / contentRect.width());
                        textRect.setWidth(textRect.width() * (skipText + 1));
                    }

                    textRect.adjust(3, 3, -3, -3);

                    painter.setBrush(Qt::transparent);
                    painter.setPen(QColor(255, 255, 255));
                    painter.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, number);
                }
            }
        } else {
            skipText--;
        }

        if (drawLine) {
            painter.setPen(QColor(255, 255, 255, 127));
            painter.drawLine(static_cast<int>(frameStart), 0, static_cast<int>(frameStart), this->height());
        }

        if (i == d->timeline->currentFrame()) {
            painter.setPen(Qt::transparent);
            painter.setBrush(QColor(255, 255, 255, 100));
            painter.drawRect(contentRect);
        }

        painter.restore();
    }
}

void TimelineRightWidgetProperty::mousePressEvent(QMouseEvent* event) {
    d->element->beginTransaction();
    quint64 frame = this->frameForPoint(event->pos().x());
    d->mouseFrameStart = frame;

    if (event->button() == Qt::LeftButton) {
        if ((event->modifiers() & Qt::ControlModifier) == 0) {
            d->timeline->clearCurrentSelection();
        }

        if (!d->property.isEmpty()) {
            TimelineElement* timelineElement = d->element->timelineElementAtFrame(d->property, frame);
            if (timelineElement) {
                d->timeline->addToCurrentSelection(timelineElement);
                if (timelineElement->startFrame() == frame) {
                    d->mouseState = TimelineRightWidgetPropertyPrivate::MouseLeadTransition;
                } else if (timelineElement->endFrame() == frame) {
                    d->mouseState = TimelineRightWidgetPropertyPrivate::MouseTailTransition;
                } else {
                    d->mouseState = TimelineRightWidgetPropertyPrivate::MouseMidTransition;
                    d->initialElementFrameStart = timelineElement->startFrame();
                }
                d->mouseTimelineElement = timelineElement;
                d->oldState = TimelineElementState(timelineElement);
            } else {
                d->mouseState = TimelineRightWidgetPropertyPrivate::MouseNotOnTransition;
            }
        } else {
            d->mouseState = TimelineRightWidgetPropertyPrivate::MouseDownNoAction;
        }
    } else {
        d->mouseState = TimelineRightWidgetPropertyPrivate::MouseDownNoAction;
    }
    d->timeline->setCurrentFrame(frame);
}

void TimelineRightWidgetProperty::mouseMoveEvent(QMouseEvent* event) {
    quint64 frame = this->frameForPoint(event->pos().x());
    switch (d->mouseState) {
        case TimelineRightWidgetPropertyPrivate::MouseDownNoAction:
            break;
        case TimelineRightWidgetPropertyPrivate::MouseIdle: {
            if (!d->property.isEmpty()) {
                TimelineElement* timelineElement = d->element->timelineElementAtFrame(d->property, frame);
                if (timelineElement) {
                    if (timelineElement->startFrame() == frame || timelineElement->endFrame() == frame) {
                        this->setCursor(QCursor(Qt::SizeHorCursor));
                    } else {
                        this->setCursor(QCursor(Qt::ArrowCursor));
                    }
                } else {
                    this->setCursor(QCursor(Qt::ArrowCursor));
                }
            }
            break;
        }
        case TimelineRightWidgetPropertyPrivate::MouseMidTransition: {
            quint64 startFrame;
            if (d->initialElementFrameStart + frame < d->mouseFrameStart) {
                startFrame = 0;
            } else {
                startFrame = d->initialElementFrameStart + frame - d->mouseFrameStart;
            }

            if (startFrame + d->mouseTimelineElement->length() > d->timeline->frameCount()) startFrame = d->timeline->frameCount() - 1 - d->mouseTimelineElement->length();

            quint64 endFrame = startFrame + d->mouseTimelineElement->length();

            d->mouseTimelineElement->setStartFrame(startFrame);
            d->mouseTimelineElement->setEndFrame(endFrame);
            break;
        }
        case TimelineRightWidgetPropertyPrivate::MouseLeadTransition:
            d->mouseTimelineElement->setStartFrame(frame);
            break;
        case TimelineRightWidgetPropertyPrivate::MouseTailTransition:
            d->mouseTimelineElement->setEndFrame(frame);
            break;
        case TimelineRightWidgetPropertyPrivate::MouseNotOnTransition:
            if (frame != d->mouseFrameStart) {
                quint64 startFrame = frame;
                quint64 endFrame = d->mouseFrameStart;

                if (endFrame < startFrame) {
                    startFrame = d->mouseFrameStart;
                    endFrame = frame;
                }

                QVariant valueAtFrameStart = d->element->propertyValueForFrame(d->property, frame);
                TimelineElement* element = new TimelineElement(startFrame, valueAtFrameStart, endFrame, valueAtFrameStart);
                d->element->addTimelineElement(d->property, element);
                d->timeline->setCurrentSelection(element);
                d->mouseTimelineElement = element;
                d->mouseState = TimelineRightWidgetPropertyPrivate::MouseCreatingTransition;
            }
            break;
        case TimelineRightWidgetPropertyPrivate::MouseCreatingTransition: {
            quint64 startFrame = frame;
            quint64 endFrame = d->mouseFrameStart;

            if (endFrame < startFrame) {
                startFrame = d->mouseFrameStart;
                endFrame = frame;
            }

            d->mouseTimelineElement->setStartFrame(startFrame);
            d->mouseTimelineElement->setEndFrame(endFrame);
        }
    }

    if (d->mouseState != TimelineRightWidgetPropertyPrivate::MouseIdle) {
        d->timeline->setCurrentFrame(frame);
    }
}

void TimelineRightWidgetProperty::mouseReleaseEvent(QMouseEvent* event) {
    if (d->element->tryCommitTransaction()) {
        QUndoCommand* undoCommand = nullptr;
        switch (d->mouseState) {
            case TimelineRightWidgetPropertyPrivate::MouseMidTransition: {
                TimelineElementState newState = TimelineElementState(d->mouseTimelineElement);
                if (newState != d->oldState) undoCommand = new UndoTimelineElementModify(tr("Move Timeline Element"), d->oldState, newState);
                break;
            }
            case TimelineRightWidgetPropertyPrivate::MouseLeadTransition:
            case TimelineRightWidgetPropertyPrivate::MouseTailTransition: {
                TimelineElementState newState = TimelineElementState(d->mouseTimelineElement);
                if (newState != d->oldState) undoCommand = new UndoTimelineElementModify(tr("Resize Timeline Element"), d->oldState, newState);
                break;
            }
            case TimelineRightWidgetPropertyPrivate::MouseCreatingTransition:
                undoCommand = new UndoNewTimelineElement(tr("New Timeline Element"), TimelineElementState(d->mouseTimelineElement));
                break;
            case TimelineRightWidgetPropertyPrivate::MouseDownNoAction:
            case TimelineRightWidgetPropertyPrivate::MouseIdle:
            case TimelineRightWidgetPropertyPrivate::MouseNotOnTransition:
                break;

        }

        if (undoCommand) {
            d->timeline->undoStack()->push(undoCommand);
        }
    }

    d->mouseState = TimelineRightWidgetPropertyPrivate::MouseIdle;
    d->mouseTimelineElement = nullptr;
}

void TimelineRightWidgetProperty::focusOutEvent(QFocusEvent* event) {
    this->update();
}

void TimelineRightWidgetProperty::timerParametersChanged() {
    this->setFixedWidth(static_cast<int>(d->timeline->frameSpacing() * static_cast<double>(d->timeline->frameCount())));
    this->update();
}

void TimelineRightWidgetProperty::draw() {
    this->update();
}
