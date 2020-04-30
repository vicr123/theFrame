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

#include "viewport/rectangleviewport.h"

struct ViewportPrivate {
    ViewportElement* rootElement;
    Prerenderer* prerenderer;
    Timeline* timeline;

    quint64 currentFrame;
    QPixmap rootRender;

    QRect viewportRect;

    QMultiMap<QObject*, QWidget*> adjustmentWidgets;

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
                switch (element->propertyType(timelineElement->propertyName())) {
                    case Element::Rect: {
                        RectangleViewport* start = new RectangleViewport(RectangleViewport::StartType, this);
                        start->setOffset(element->parentElement()->renderOffset(timelineElement->startFrame()));
                        start->setValue(timelineElement->startValue().toRect());
                        connect(start, &RectangleViewport::valueChanged, this, [=](QRect value) {
                            TimelineElementState oldState(timelineElement);
                            timelineElement->setStartValue(value);
                            d->timeline->undoStack()->push(new UndoTimelineElementModify(tr("Start Value Change"), oldState, TimelineElementState(timelineElement)));
                        });
                        connect(start, &RectangleViewport::focusFrame, this, [=] {
                            d->timeline->setCurrentFrame(timelineElement->startFrame());
                        });

                        RectangleViewport* end = new RectangleViewport(RectangleViewport::EndType, this);
                        end->setOffset(element->parentElement()->renderOffset(timelineElement->endFrame()));
                        end->setValue(timelineElement->endValue().toRect());
                        connect(end, &RectangleViewport::valueChanged, this, [=](QRect value) {
                            TimelineElementState oldState(timelineElement);
                            timelineElement->setEndValue(value);
                            d->timeline->undoStack()->push(new UndoTimelineElementModify(tr("End Value Change"), oldState, TimelineElementState(timelineElement)));
                        });
                        connect(end, &RectangleViewport::focusFrame, this, [=] {
                            d->timeline->setCurrentFrame(timelineElement->endFrame());
                        });

                        connect(timelineElement, &TimelineElement::elementPropertyChanged, start, [=] {
                            QSignalBlocker blocker(start);
                            start->setValue(timelineElement->startValue().toRect());
                            start->setOffset(element->parentElement()->renderOffset(timelineElement->startFrame()));
                        });
                        connect(timelineElement, &TimelineElement::elementPropertyChanged, end, [=] {
                            QSignalBlocker blocker(end);
                            end->setValue(timelineElement->endValue().toRect());
                            end->setOffset(element->parentElement()->renderOffset(timelineElement->endFrame()));
                        });

                        d->adjustmentWidgets.insert(timelineElement, start);
                        d->adjustmentWidgets.insert(timelineElement, end);
                        break;
                    }
                    case Element::Integer:
                    case Element::Double:
                    case Element::Color:
                    case Element::Percentage:
                    case Element::String:
                    case Element::Point:
                    case Element::Font:
                    case Element::File:
                        break;
                }
            }
        }
    }

    for (QObject* w : toDelete) {
        for (QWidget* widget : d->adjustmentWidgets.values(w)) {
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
