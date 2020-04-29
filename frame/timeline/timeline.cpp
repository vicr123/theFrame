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
#include "timeline.h"
#include "ui_timeline.h"

#include <QScrollBar>
#include <QScroller>
#include <QPointer>
#include <QJsonObject>
#include <QJsonArray>
#include <elements/timelineelement.h>
#include <the-libs_global.h>

#include "undo/undodeletetimelineelement.h"
#include "undo/undodeleteelement.h"

#include "timelineleftwidget.h"
#include "timelinerightwidget.h"

struct TimelinePrivate {
    TimelineLeftWidget* rootLeftWidget;
    ViewportElement* rootViewportElement;
    Prerenderer* prerenderer;

    QUndoStack* undoStack;
    QList<QObject*> currentSelection;

    quint64 frameCount = 1200;
    double frameSpacing = 1;
    uint framerate = 60;
    quint64 currentFrame = 0;

    quint64 inPoint = 0;
    quint64 outPoint = 0;
};

Timeline::Timeline(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::Timeline) {
    ui->setupUi(this);

    d = new TimelinePrivate();
    ui->timelineLeftPane->setFixedWidth(SC_DPI(300));
//    this->setMaximumHeight(SC_DPI(500));

    connect(ui->timelineLeftPane->verticalScrollBar(), &QScrollBar::valueChanged, ui->timelineRightPane->verticalScrollBar(), &QScrollBar::setValue);
    connect(ui->timelineRightPane->verticalScrollBar(), &QScrollBar::valueChanged, ui->timelineLeftPane->verticalScrollBar(), &QScrollBar::setValue);

    ui->timelineLeftPane->setProperty("X-Contemporary-NoInstallScroller", true);
    ui->timelineRightPane->setProperty("X-Contemporary-NoInstallScroller", true);
}

Timeline::~Timeline() {
    delete ui;
}

void Timeline::setUndoStack(QUndoStack* undoStack)
{
    d->undoStack = undoStack;
}

QUndoStack* Timeline::undoStack()
{
    return d->undoStack;
}

void Timeline::setViewportElement(ViewportElement* element) {
    d->rootViewportElement = element;
    d->rootLeftWidget = new TimelineLeftWidget(this, element, true);
    ui->leftPaneLayout->addWidget(d->rootLeftWidget);
    ui->rightPaneLayout->addWidget(d->rootLeftWidget->rightWidget());
}

ViewportElement* Timeline::viewportElement() {
    return d->rootViewportElement;
}

void Timeline::setPrerenderer(Prerenderer* prerenderer) {
    d->prerenderer = prerenderer;
}

Prerenderer* Timeline::prerenderer() const {
    return d->prerenderer;
}

void Timeline::setFrameSpacing(double frameSpacing) {
    d->frameSpacing = frameSpacing;
    emit frameSpacingChanged(frameSpacing);
}

double Timeline::frameSpacing() const {
    return d->frameSpacing;
}

void Timeline::setFrameCount(quint64 frameCount) {
    if (d->outPoint > frameCount - 1) this->setOutPoint(frameCount - 1);
    d->frameCount = frameCount;
    emit frameCountChanged(frameCount);
}

quint64 Timeline::frameCount() const {
    return d->frameCount;
}

void Timeline::setFramerate(uint framerate) {
    d->framerate = framerate;
    emit framerateChanged(framerate);
}

uint Timeline::framerate() const {
    return d->framerate;
}

void Timeline::setCurrentFrame(quint64 frame) {
    if (frame >= frameCount()) frame = frameCount() - 1;
    d->currentFrame = frame;
    emit currentFrameChanged(frame);
}

quint64 Timeline::currentFrame() const {
    return d->currentFrame;
}

void Timeline::setCurrentSelection(QObject* element) {
    clearCurrentSelection();
    this->addToCurrentSelection(element);
}

void Timeline::addToCurrentSelection(QObject* element) {
    if (d->currentSelection.count() > 0) {
        if (strcmp(element->metaObject()->className(), d->currentSelection.first()->metaObject()->className()) != 0) clearCurrentSelection();
    }

    auto deleteFunction = [ = ] {
        d->currentSelection.removeOne(element);
        emit currentSelectionChanged();
    };

    if (element->metaObject()->inherits(&TimelineElement::staticMetaObject)) {
        connect(qobject_cast<TimelineElement*>(element), &TimelineElement::aboutToDelete, this, deleteFunction);
    } else {
        connect(element, &QObject::destroyed, this, deleteFunction);
    }
    d->currentSelection.append(element);
    emit currentSelectionChanged();
}

void Timeline::clearCurrentSelection() {
    d->currentSelection.clear();
    emit currentSelectionChanged();
}

QList<QObject*> Timeline::currentSelection() {
    return d->currentSelection;
}

void Timeline::deleteSelected() {

    d->undoStack->beginMacro(tr("Delete"));
    for (QObject* element : d->currentSelection) {
        //Make sure we're not deleting the root viewport
        if (element == d->rootLeftWidget) continue;

        QUndoCommand* undoCommand = nullptr;
        if (element->metaObject()->inherits(&TimelineElement::staticMetaObject)) {
            undoCommand = new UndoDeleteTimelineElement(tr("Delete Timeline Element"), TimelineElementState(static_cast<TimelineElement*>(element)));
        } else if (element->metaObject()->inherits(&Element::staticMetaObject)) {
            Element* e = static_cast<Element*>(element);
            undoCommand = new UndoDeleteElement(tr("Delete %1 \"%2\"").arg(e->typeDisplayName()).arg(e->name()), ElementState(e));
        }

        if (undoCommand) d->undoStack->push(undoCommand);

        element->deleteLater();
    }
    d->undoStack->endMacro();
    d->currentSelection.clear();
    emit currentSelectionChanged();
}

void Timeline::setInPoint(quint64 inPoint)
{
    if (d->inPoint == 0 && d->outPoint == 0) d->outPoint = d->frameCount - 1;
    if (inPoint > d->outPoint) {
        d->inPoint = d->outPoint;
        d->outPoint = inPoint;
    } else {
        d->inPoint = inPoint;
    }
    emit inOutPointChanged(d->inPoint, d->outPoint);
}

quint64 Timeline::inPoint()
{
    return d->inPoint;
}

void Timeline::setOutPoint(quint64 outPoint)
{
    if (d->inPoint == 0 && d->outPoint == 0) d->inPoint = 0;
    if (outPoint < d->inPoint) {
        d->outPoint = d->inPoint;
        d->inPoint = outPoint;
    } else {
        d->outPoint = outPoint;
    }
    emit inOutPointChanged(d->inPoint, d->outPoint);
}

quint64 Timeline::outPoint()
{
    return d->outPoint;
}

void Timeline::clearInOutPoint()
{
    d->inPoint = 0;
    d->outPoint = 0;
    emit inOutPointChanged(0, 0);
}

bool Timeline::isInPreviewRange(quint64 frame)
{
    if (d->inPoint == 0 && d->outPoint == 0) return false;
    return d->inPoint <= frame && d->outPoint >= frame;
}

QJsonObject Timeline::save() const {
    QJsonObject rootObj;
    rootObj.insert("framerate", static_cast<qint64>(this->framerate()));
    rootObj.insert("frameCount", QString::number(this->frameCount()));
    rootObj.insert("frameSpacing", this->frameSpacing());

    QJsonArray viewportSize = {
        d->rootViewportElement->viewportSize().width(),
        d->rootViewportElement->viewportSize().height()
    };
    rootObj.insert("viewportSize", viewportSize);
    rootObj.insert("viewport", d->rootViewportElement->save());
    return rootObj;
}

bool Timeline::load(QJsonObject obj) {
    this->setFramerate(static_cast<uint>(obj.value("framerate").toInt()));
    this->setFrameCount(obj.value("frameCount").toString().toULongLong());
    this->setFrameSpacing(obj.value("frameSpacing").toDouble());

    QJsonArray viewportSize = obj.value("viewportSize").toArray();
    d->rootViewportElement->setViewportSize(QSize(viewportSize.at(0).toInt(), viewportSize.at(1).toInt()));

    d->rootViewportElement->load(obj.value("viewport").toObject());

    return true;
}
