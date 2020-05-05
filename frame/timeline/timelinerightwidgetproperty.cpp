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

#include <QClipboard>
#include <QMimeData>
#include <ttoast.h>
#include <tsystemsound.h>

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
    QList<quint64> initialElementFrameStart;
    QPointer<TimelineElement> mouseTimelineElement;
    QList<TimelineElementState> oldState;
    QElapsedTimer lastClickEvent;
    TimelineElement* lastClickTimelineElement;
    int numClicks = 0;
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

void TimelineRightWidgetProperty::paste()
{
    const QMimeData* mimeData = QApplication::clipboard()->mimeData();
    QJsonArray list = QJsonDocument::fromBinaryData(mimeData->data("application/x-theframe-timelineelements")).array();

    // Figure out how to paste this data
    // 1. If there is only one element, and it is of the same property type, paste that element here

    if (!d->property.isEmpty() &&
            list.count() == 1 &&
            list.first().toObject().value("type").toInt() == d->element->propertyType(d->property)) {
        d->element->beginTransaction();

        TimelineElement* element = new TimelineElement(d->element);
        element->load(list.first().toObject());
        element->moveStartFrame(d->timeline->currentFrame());
        d->element->addTimelineElement(d->property, element);

        if (d->element->tryCommitTransaction()) {
            d->timeline->setCurrentSelection(d->element);
            d->timeline->undoStack()->push(new UndoNewTimelineElement(tr("Paste Timeline Element"), TimelineElementState(element)));
        } else {
            tToast* toast = new tToast(this);
            toast->setTitle(tr("Paste"));
            toast->setText(tr("Can't paste that timeline element here because it would overlap with other timeline elements"));
            toast->show(this->window());
            connect(toast, &tToast::dismissed, toast, &tToast::deleteLater);
            tSystemSound::play("bell");
        }
        return;
    }

    // 2. If the parent element contains properties of the same name (with matching types) paste the elements there
    {
        bool isOk = true;
        int earliestFrameIndex = 0;
        for (int i = 0; i < list.count(); i++) {
            QJsonObject elementObject = list.at(i).toObject();
            if (d->element->allProperties().contains(elementObject.value("property").toString()) &&
                d->element->allProperties().value(elementObject.value("property").toString()) == d->element->propertyType(elementObject.value("property").toString())) {

                if (list.at(earliestFrameIndex).toObject().value("startFrame").toString().toULongLong() > elementObject.value("startFrame").toString().toULongLong())
                    earliestFrameIndex = i;
            } else {
                isOk = false;
            }
        }

        if (isOk) {
            d->element->beginTransaction();

            quint64 earliestFrame = list.at(earliestFrameIndex).toObject().value("startFrame").toString().toULongLong();
            QList<TimelineElement*> elements;
            for (QJsonValue elementValue : list) {
                QJsonObject elementObject = elementValue.toObject();
                TimelineElement* element = new TimelineElement(d->element);
                element->load(elementObject);
                element->moveStartFrame(d->timeline->currentFrame() + element->startFrame() - earliestFrame);
                d->element->addTimelineElement(element->propertyName(), element);
                elements.append(element);
            }

            if (d->element->tryCommitTransaction()) {
                d->timeline->clearCurrentSelection();
                QList<TimelineElementState> elementStates;
                for (TimelineElement* element : elements) {
                    d->timeline->addToCurrentSelection(element);
                    elementStates.append(TimelineElementState(element));
                }
                d->timeline->undoStack()->push(new UndoNewTimelineElement(tr("Paste Timeline Elements"), elementStates));
            } else {
                tToast* toast = new tToast(this);
                toast->setTitle(tr("Paste"));
                toast->setText(tr("Can't paste those timeline elements here because they would overlap with other timeline elements"));
                toast->show(this->window());
                connect(toast, &tToast::dismissed, toast, &tToast::deleteLater);
                tSystemSound::play("bell");
            }
            return;
        }
    }

    // 3. TODO: If the parent element contains properties of the same types, paste the elements there

    // 4. This is an invalid paste operation. Work out what went wrong and try to show a helpful toast
    tToast* toast = new tToast(this);
    toast->setTitle(tr("Paste"));

    if (!d->property.isEmpty() &&
            list.count() == 1 &&
            list.first().toObject().value("type").toInt() != d->element->propertyType(d->property)) {
        //The user is trying to paste, but the types don't match
        toast->setText(tr("Can't paste that timeline element here because the type of the timeline element you're pasting doesn't match"));
    } else {
        toast->setText(tr("Can't paste those timeline elements here"));
    }

    toast->show(this->window());
    connect(toast, &tToast::dismissed, toast, &tToast::deleteLater);
    tSystemSound::play("bell");
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
    for (quint64 i = d->timeline->leftRenderFrame(); i < d->timeline->rightRenderFrame(); i++) {
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
    int skipText = 0;
    for (quint64 i = d->timeline->leftRenderFrame(); i < d->timeline->rightRenderFrame(); i++) {
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
    quint64 frame = this->frameForPoint(event->pos().x());
    TimelineElement* timelineElement = d->element->timelineElementAtFrame(d->property, frame);
    if (d->lastClickTimelineElement != timelineElement) {
        d->numClicks = 1;
        d->lastClickEvent.start();
    } else {
        if (d->lastClickEvent.restart() < QApplication::doubleClickInterval()) {
            d->numClicks++;
        } else {
            d->numClicks = 1;
        }
    }
    d->lastClickTimelineElement = timelineElement;

    switch (d->numClicks) {
        case 1:
            singleClick(event);
            break;
        case 2:
            doubleClick(event);
            break;
        case 3:
            tripleClick(event);
            break;
    }
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
            quint64 mouseFrameStart = d->mouseFrameStart;
            for (int i = 0; i < d->initialElementFrameStart.count(); i++) {
                if (d->initialElementFrameStart.at(i) + frame < mouseFrameStart) {
                    mouseFrameStart = d->initialElementFrameStart.at(i) + frame;
                }

//                if (startFrame + element->length() > d->timeline->frameCount()) startFrame = d->timeline->frameCount() - 1 - element->length();
            }

            for (int i = 0; i < d->initialElementFrameStart.count(); i++) {
                static_cast<TimelineElement*>(d->timeline->currentSelection().at(i))->moveStartFrame(d->initialElementFrameStart.at(i) + frame - mouseFrameStart);
            }
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
    bool shouldCommit = true;
    QList<Element*> elements;
    elements.append(d->element);
    for (QObject* o : d->timeline->currentSelection()) {
        TimelineElement* element = static_cast<TimelineElement*>(o);
        if (!elements.contains(element->parentElement())) elements.append(element->parentElement());
        if (!element->parentElement()->canCommitTransaction()) shouldCommit = false;
    }

    if (shouldCommit) {
        for (Element* element : elements) {
            element->tryCommitTransaction();
        }

        QUndoCommand* undoCommand = nullptr;
        switch (d->mouseState) {
            case TimelineRightWidgetPropertyPrivate::MouseMidTransition: {
                QList<TimelineElementState> newStates;
                for (QObject* o : d->timeline->currentSelection()) {
                    TimelineElement* element = static_cast<TimelineElement*>(o);
                    newStates.append(TimelineElementState(element));
                }
                if (newStates != d->oldState) undoCommand = new UndoTimelineElementModify(tr("Move Timeline Element"), d->oldState, newStates);
                break;
            }
            case TimelineRightWidgetPropertyPrivate::MouseLeadTransition:
            case TimelineRightWidgetPropertyPrivate::MouseTailTransition: {
                QList<TimelineElementState> newState({TimelineElementState(d->mouseTimelineElement)});
                if (newState != d->oldState) undoCommand = new UndoTimelineElementModify(tr("Resize Timeline Element"), d->oldState, {newState});
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
    } else {
        for (Element* element : elements) {
            element->rollbackTransaction();
        }
    }

    d->mouseState = TimelineRightWidgetPropertyPrivate::MouseIdle;
    d->mouseTimelineElement = nullptr;
}

void TimelineRightWidgetProperty::mouseDoubleClickEvent(QMouseEvent* event)
{
    this->mousePressEvent(event);
}

void TimelineRightWidgetProperty::focusOutEvent(QFocusEvent* event) {
    this->update();
}

void TimelineRightWidgetProperty::singleClick(QMouseEvent* event)
{
    quint64 frame = this->frameForPoint(event->pos().x());
    d->mouseFrameStart = frame;

    auto clearSelection = [=] {
        if ((event->modifiers() & Qt::ControlModifier) == 0) {
            d->timeline->clearCurrentSelection();
        }
    };

    if (event->button() == Qt::LeftButton) {
        if (!d->property.isEmpty()) {
            TimelineElement* timelineElement = d->element->timelineElementAtFrame(d->property, frame);
            if (timelineElement) {
                if (!d->timeline->currentSelection().contains(timelineElement)) {
                    clearSelection();
                }

                d->timeline->addToCurrentSelection(timelineElement);

                if (timelineElement->startFrame() == frame) {
                    d->mouseState = TimelineRightWidgetPropertyPrivate::MouseLeadTransition;
                } else if (timelineElement->endFrame() == frame) {
                    d->mouseState = TimelineRightWidgetPropertyPrivate::MouseTailTransition;
                } else {
                    d->mouseState = TimelineRightWidgetPropertyPrivate::MouseMidTransition;
                    d->initialElementFrameStart.clear();
                    for (QObject* o : d->timeline->currentSelection()) {
                        TimelineElement* element = static_cast<TimelineElement*>(o);
                        d->initialElementFrameStart.append(element->startFrame());
                    }
                }
                d->mouseTimelineElement = timelineElement;

                d->oldState.clear();
                for (QObject* o : d->timeline->currentSelection()) {
                    TimelineElement* element = static_cast<TimelineElement*>(o);
                    d->oldState.append(TimelineElementState(element));
                }
            } else {
                d->mouseState = TimelineRightWidgetPropertyPrivate::MouseNotOnTransition;
                clearSelection();
            }
        } else {
            d->mouseState = TimelineRightWidgetPropertyPrivate::MouseDownNoAction;
            clearSelection();
        }

        QList<Element*> elements;
        elements.append(d->element);
        for (QObject* o : d->timeline->currentSelection()) {
            TimelineElement* element = static_cast<TimelineElement*>(o);
            if (!elements.contains(element->parentElement())) elements.append(element->parentElement());
        }
        for (Element* element : elements) element->beginTransaction();
        d->timeline->setSelectedTimelineRightWidget(this);
    } else {
        d->mouseState = TimelineRightWidgetPropertyPrivate::MouseDownNoAction;
    }
    d->timeline->setCurrentFrame(frame);
}

void TimelineRightWidgetProperty::doubleClick(QMouseEvent* event)
{
    if (d->property == "") return;
    //Select all timeline elements on the current property
    d->timeline->clearCurrentSelection();
    for (TimelineElement* element : d->element->timelineElements(d->property)) {
        d->timeline->addToCurrentSelection(element);
    }
}

void TimelineRightWidgetProperty::tripleClick(QMouseEvent* event)
{
    //Select all timeline elements for this element
    d->timeline->clearCurrentSelection();
    for (QString property : d->element->allProperties().keys()) {
        for (TimelineElement* element : d->element->timelineElements(property)) {
            d->timeline->addToCurrentSelection(element);
        }
    }
}

void TimelineRightWidgetProperty::timerParametersChanged() {
    this->setFixedWidth(static_cast<int>(d->timeline->frameSpacing() * static_cast<double>(d->timeline->frameCount())));
    this->update();
}

void TimelineRightWidgetProperty::draw() {
    this->update();
}
