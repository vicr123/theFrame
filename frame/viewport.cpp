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
#include "viewport.h"
#include "ui_viewport.h"

#include <QMap>
#include "timeline/timeline.h"
#include "prerenderer.h"

#include <elements/element.h>
#include <elements/timelineelement.h>

#include <QUndoStack>
#include "undo/undotimelineelementmodify.h"
#include "undo/undoelementmodify.h"

#include "viewport/viewportproperty.h"

struct ViewportPrivate {
    ViewportElement* rootElement;
    Prerenderer* prerenderer;
    Timeline* timeline;

    quint64 currentFrame;
    QPixmap rootRender;

    QRect viewportRect;

    QMultiMap<QObject*, ViewportProperty*> adjustmentWidgets;

    bool renderingFrame = false;
    bool renderRequired = false;
};

Viewport::Viewport(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::Viewport) {
    ui->setupUi(this);

    d = new ViewportPrivate();
    d->rootElement = new ViewportElement();
    d->rootElement->setName(tr("Canvas"));

    connect(d->rootElement, &ViewportElement::invalidateFromFrame, this, [=](quint64 frame) {
        if (d->currentFrame >= frame) this->setFrame(d->currentFrame);
    });
    connect(d->rootElement, &ViewportElement::viewportSizeChanged, this, &Viewport::updateViewportRect);
}

Viewport::~Viewport() {
    d->rootElement->deleteLater();
    delete ui;
    delete d;
}

void Viewport::setFrame(quint64 frame) {
    d->currentFrame = frame;

    if (!d->renderingFrame) {
        d->renderingFrame = true;
        d->prerenderer->frame(frame)->then([ = ](QPixmap pixmap) {
            d->rootRender = pixmap;

            //Force a repaint
            this->repaint();

            d->renderingFrame = false;
            if (d->renderRequired) {
                d->renderRequired = false;
                this->setFrame(d->currentFrame);
            }
        });
    } else {
        d->renderRequired = true;
    }
}

void Viewport::setPrerenderer(Prerenderer* prerenderer) {
    d->prerenderer = prerenderer;
    connect(prerenderer, &Prerenderer::framePrerenderStateChanged, this, [ = ](quint64 frame) {
        if (frame == d->currentFrame) setFrame(frame);
    });
}

void Viewport::setTimeline(Timeline* timeline)
{
    d->timeline = timeline;
    connect(timeline, &Timeline::currentSelectionChanged, this, &Viewport::updateTimelineSelection);
}

Timeline* Viewport::timeline()
{
    return d->timeline;
}

ViewportElement* Viewport::rootElement() {
    return d->rootElement;
}

QRect Viewport::viewportRect()
{
    return d->viewportRect;
}

double Viewport::viewportScale()
{
    return d->viewportRect.width() / static_cast<double>(d->rootElement->viewportSize().width());
}

QRect Viewport::viewportToCanvas(QRect viewportCoordinates)
{
    double scale = this->viewportScale();

    QPoint newPoint = (viewportCoordinates.topLeft() - viewportRect().topLeft()) / scale;
    QSize newSize = viewportCoordinates.size() / scale;

    return QRect(newPoint, newSize);
}

QRect Viewport::canvasToViewport(QRect canvasCoordinates)
{
    double scale = this->viewportScale();

    QPoint newPoint = canvasCoordinates.topLeft() * scale + viewportRect().topLeft();
    QSize newSize = canvasCoordinates.size() * scale;

    return QRect(newPoint, newSize);
}

void Viewport::updateTimelineSelection()
{
    QList<QObject*> toDelete = d->adjustmentWidgets.keys();

    if (d->timeline->currentSelection().count() > 0) {
        TimelineElement* timelineElement = qobject_cast<TimelineElement*>(d->timeline->currentSelection().first());
        if (timelineElement) {
            if (d->adjustmentWidgets.contains(timelineElement)) {
                toDelete.removeOne(timelineElement);
            } else {
                Element* element = timelineElement->parentElement();

                ViewportProperty* start = ViewportProperty::constructForType(element->propertyType(timelineElement->propertyName()), ViewportProperty::StartType, this);
                ViewportProperty* end = ViewportProperty::constructForType(element->propertyType(timelineElement->propertyName()), ViewportProperty::EndType, this);

                if (start) {
                    start->setOffset(element->parentElement()->renderOffset(timelineElement->startFrame()));
                    start->setValue(timelineElement->startValue());
                    start->setAnchored(timelineElement->startAnchored());
                    connect(start, &ViewportProperty::valueChanged, this, [=](QVariant value) {
                        TimelineElementState oldState(timelineElement);
                        timelineElement->setStartValue(value);
                        d->timeline->undoStack()->push(new UndoTimelineElementModify(tr("Start Value Change"), oldState, TimelineElementState(timelineElement)));
                    });
                    connect(start, &ViewportProperty::focusFrame, this, [=] {
                        d->timeline->setCurrentFrame(timelineElement->startFrame());
                    });
                    connect(timelineElement, &TimelineElement::elementPropertyChanged, start, [=] {
                        QSignalBlocker blocker(start);
                        start->setValue(timelineElement->startValue());
                        start->setOffset(element->parentElement()->renderOffset(timelineElement->startFrame()));
                        start->setAnchored(timelineElement->startAnchored());
                    });
                    d->adjustmentWidgets.insert(timelineElement, start);
                }

                if (end) {
                    end->setOffset(element->parentElement()->renderOffset(timelineElement->endFrame()));
                    end->setValue(timelineElement->endValue());
                    connect(end, &ViewportProperty::valueChanged, this, [=](QVariant value) {
                        TimelineElementState oldState(timelineElement);
                        timelineElement->setEndValue(value);
                        d->timeline->undoStack()->push(new UndoTimelineElementModify(tr("End Value Change"), oldState, TimelineElementState(timelineElement)));
                    });
                    connect(end, &ViewportProperty::focusFrame, this, [=] {
                        d->timeline->setCurrentFrame(timelineElement->endFrame());
                    });

                    connect(timelineElement, &TimelineElement::elementPropertyChanged, end, [=] {
                        QSignalBlocker blocker(end);
                        end->setValue(timelineElement->endValue());
                        end->setOffset(element->parentElement()->renderOffset(timelineElement->endFrame()));
                    });

                    d->adjustmentWidgets.insert(timelineElement, end);
                }
            }
        }

        Element* element = qobject_cast<Element*>(d->timeline->currentSelection().first());
        if (element) {
            if (d->adjustmentWidgets.contains(element)) {
                toDelete.removeOne(element);
            } else {
                for (QString property : element->allProperties().keys()) {
                    ViewportProperty* prop = ViewportProperty::constructForType(element->propertyType(property), ViewportProperty::StartValueType, this);
                    if (prop) {
                        prop->setOffset(element->renderOffset(0));
                        prop->setValue(element->startValue(property));
                        connect(prop, &ViewportProperty::valueChanged, this, [=](QVariant value) {
                            ElementState oldState(element);
                            element->setStartValue(property, value);
                            d->timeline->undoStack()->push(new UndoElementModify(tr("Start Value Change"), oldState, ElementState(element)));
                        });
                        connect(prop, &ViewportProperty::focusFrame, this, [=] {
                            d->timeline->setCurrentFrame(0);
                        });

                        connect(element, &Element::startValueChanged, prop, [=](QString propertyChanged, QVariant value) {
                            if (property == propertyChanged) {
                                QSignalBlocker blocker(prop);
                                prop->setValue(value);
                            }
                        });

                        d->adjustmentWidgets.insert(element, prop);
                    }
                }

            }
        }
    }

    for (QObject* w : toDelete) {
        for (ViewportProperty* widget : d->adjustmentWidgets.values(w)) {
            widget->deleteLater();
        }
        d->adjustmentWidgets.remove(w);
    }
}

void Viewport::updateViewportRect()
{
    QSize viewportSize = d->rootElement->viewportSize();
    QSize drawSize = viewportSize.scaled(this->width(), this->height(), Qt::KeepAspectRatio);

    QRect drawRect;
    drawRect.setSize(drawSize);
    drawRect.moveCenter(QPoint(this->width() / 2, this->height() / 2));
    d->viewportRect = drawRect;

    emit viewportRectChanged();
    this->update();
}

void Viewport::paintEvent(QPaintEvent* event) {
    QPainter painter(this);

    //Draw the picture
    painter.drawPixmap(d->viewportRect, d->rootRender);
}

void Viewport::resizeEvent(QResizeEvent* event)
{
    updateViewportRect();
}
