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
#include <QMimeData>
#include <QClipboard>
#include <QJsonDocument>
#include <QPinchGesture>
#include <tsystemsound.h>
#include <elements/timelineelement.h>
#include <the-libs_global.h>
#include <math.h>

#include "undo/undodeletetimelineelement.h"
#include "undo/undodeleteelement.h"
#include "undo/undonewelement.h"

#include "timelineleftwidget.h"
#include "timelinerightwidget.h"
#include "timelinerightwidgetproperty.h"

struct TimelinePrivate {
    TimelineLeftWidget* rootLeftWidget;
    ViewportElement* rootViewportElement;
    Prerenderer* prerenderer;
    TutorialEngine* tutorialEngine;
    QWidget* timeTicker;

    QUndoStack* undoStack;
    QList<QObject*> currentSelection;

    QPointer<TimelineRightWidgetProperty> currentRightWidget;

    quint64 frameCount = 1200;
    double frameSpacing = 1;
    uint framerate = 60;
    quint64 currentFrame = 0;

    quint64 inPoint = 0;
    quint64 outPoint = 0;

    const int fileVersion = 0;
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
    connect(ui->timelineRightPane->verticalScrollBar(), &QScrollBar::valueChanged, this, [=](int value) {
        //Keep the ticker visible on top
        d->timeTicker->move(0, value);
    });

    ui->timelineLeftPane->setProperty("X-Contemporary-NoInstallScroller", true);
    ui->timelineRightPane->setProperty("X-Contemporary-NoInstallScroller", true);

    connect(QApplication::clipboard(), &QClipboard::dataChanged, this, &Timeline::canPasteChanged);

    ui->timelineRightPane->viewport()->installEventFilter(this);
    ui->timelineRightPane->viewport()->grabGesture(Qt::PinchGesture);
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

    TimelineRightWidget* rightWidget = d->rootLeftWidget->rightWidget();
    ui->rightPaneLayout->addWidget(rightWidget);
    d->timeTicker = rightWidget->mainPropertyWidget();
    d->timeTicker->setParent(ui->rightPaneContents);
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

void Timeline::setTutorialEngine(TutorialEngine* engine)
{
    d->tutorialEngine = engine;
}

TutorialEngine* Timeline::tutorialEngine()
{
    return d->tutorialEngine;
}

void Timeline::setFrameSpacing(double frameSpacing, int centerX) {
    int frameNumber = qRound((ui->timelineRightPane->horizontalScrollBar()->value() + centerX) / d->frameSpacing);
    d->frameSpacing = frameSpacing;

    int newValue = frameNumber * frameSpacing - centerX;
    ui->timelineRightPane->horizontalScrollBar()->setValue(newValue);

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

quint64 Timeline::leftRenderFrame()
{
    return ui->timelineRightPane->horizontalScrollBar()->value() / d->frameSpacing;
}

quint64 Timeline::rightRenderFrame()
{
    return qMin<quint64>(leftRenderFrame() + ui->timelineRightPane->width() / d->frameSpacing + 1, d->frameCount);
}

void Timeline::ensurePlayheadVisible()
{
    if (leftRenderFrame() < d->currentFrame && rightRenderFrame() > d->currentFrame) return;
    int newValue = d->currentFrame * d->frameSpacing - this->width() / 2;
    if (newValue < 0) newValue = 0;
    ui->timelineRightPane->horizontalScrollBar()->setValue(newValue);
}

int Timeline::playheadXPos()
{
    return d->currentFrame * d->frameSpacing - ui->timelineRightPane->horizontalScrollBar()->value();
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
        QList<const QMetaObject*> parents({
            &Element::staticMetaObject,
            &TimelineElement::staticMetaObject
        });


        bool ok = false;
        for (const QMetaObject* obj : parents) {
            if (element->metaObject()->inherits(obj) && d->currentSelection.first()->metaObject()->inherits(obj)) ok = true;
        }
        if (!ok) clearCurrentSelection();
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

    emit canCopyChanged();
    emit canCutChanged();
}

void Timeline::clearCurrentSelection() {
    d->currentSelection.clear();
    emit currentSelectionChanged();

    emit canCopyChanged();
    emit canCutChanged();
}

QList<QObject*> Timeline::currentSelection() {
    return d->currentSelection;
}

void Timeline::setSelectedTimelineRightWidget(TimelineRightWidgetProperty* rightWidget)
{
    d->currentRightWidget = rightWidget;
}

TimelineRightWidgetProperty* Timeline::selectedTimelineRightWidget()
{
    return d->currentRightWidget;
}

void Timeline::deleteSelected(QString undoText) {

    d->undoStack->beginMacro(tr("Delete"));
    for (QObject* element : d->currentSelection) {
        //Make sure we're not deleting the root viewport
        if (element == d->rootViewportElement) continue;

        QUndoCommand* undoCommand = nullptr;
        if (element->metaObject()->inherits(&TimelineElement::staticMetaObject)) {
            QString undoDescription = undoText;
            if (undoDescription == "") undoDescription = tr("Delete Timeline Element");
            undoCommand = new UndoDeleteTimelineElement(undoText, TimelineElementState(static_cast<TimelineElement*>(element)));
        } else if (element->metaObject()->inherits(&Element::staticMetaObject)) {
            QString undoDescription = undoText;
            Element* e = static_cast<Element*>(element);
            if (undoDescription == "") undoDescription = tr("Delete %1 \"%2\"").arg(e->typeDisplayName()).arg(e->name());
            undoCommand = new UndoDeleteElement(undoDescription, ElementState(e));
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

bool Timeline::canCopy()
{
    return !d->currentSelection.isEmpty();
}

void Timeline::copy()
{
    QMimeData* mimeData = this->selectedMimeData();
    if (mimeData) {
        QApplication::clipboard()->setMimeData(mimeData);
    }
}

bool Timeline::canCut()
{
    return canCopy();
}

void Timeline::cut()
{
    copy();
    deleteSelected(tr("Cut"));
}

bool Timeline::canPaste()
{
    const QMimeData* mimeData = QApplication::clipboard()->mimeData();
    if (mimeData->hasFormat("application/x-theframe-elements") || mimeData->hasFormat("application/x-theframe-timelineelements")) {
        return true;
    } else {
        return false;
    }
}

void Timeline::paste()
{
    const QMimeData* mimeData = QApplication::clipboard()->mimeData();
    if (mimeData->hasFormat("application/x-theframe-elements")) {
        QObject* firstSelection;
        if (d->currentSelection.isEmpty()) {
            firstSelection = d->rootViewportElement;
        } else {
            firstSelection = d->currentSelection.first();
        }

        if (!firstSelection->metaObject()->inherits(&Element::staticMetaObject)) return;

        int insertIndex;
        Element* e = static_cast<Element*>(firstSelection);
        if (e == d->rootViewportElement) {
            insertIndex = d->rootViewportElement->childElements().count();
        } else {
            insertIndex = e->parentElement()->childElements().indexOf(e) + 1;
            e = e->parentElement();
        }
        QJsonArray list = QJsonDocument::fromBinaryData(mimeData->data("application/x-theframe-elements")).array();

        QList<ElementState> undoList;
        for (QJsonValue elementValue : list) {
            QJsonObject elementObject = elementValue.toObject();
            Element* element = Element::constructByType(elementObject.value("type").toString());
            element->load(elementObject, false);
            e->insertChild(insertIndex, element);

            undoList.append(ElementState(element));
        }
        d->undoStack->push(new UndoNewElement(tr("Paste Elements"), undoList));
    } else if (mimeData->hasFormat("application/x-theframe-timelineelements")) {
        if (d->currentRightWidget) {
            d->currentRightWidget->paste();
        } else {
            tSystemSound::play("bell");
        }
    }
}

void Timeline::selectAll()
{
    d->currentSelection.clear();
    QList<Element*> elements({d->rootViewportElement});
    for (int i = 0; i < elements.count(); i++) {
        for (Element* child : elements.at(i)->childElements()) {
            elements.append(child);
        }
    }

    for (Element* element : elements) {
        for (QString property : element->animatableProperties().keys()) {
            for (TimelineElement* timelineElement : element->timelineElements(property)) {
                addToCurrentSelection(timelineElement);
            }
        }
    }
}

QJsonObject Timeline::save() const {
    QJsonObject rootObj;
    rootObj.insert("fileVersion", d->fileVersion);
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
    int fileVersion = obj.value("fileVersion").toInt();

    //TODO: Do some checking on the file version

    this->setFramerate(static_cast<uint>(obj.value("framerate").toInt()));
    this->setFrameCount(obj.value("frameCount").toString().toULongLong());
    this->setFrameSpacing(obj.value("frameSpacing").toDouble());

    QJsonArray viewportSize = obj.value("viewportSize").toArray();
    d->rootViewportElement->setViewportSize(QSize(viewportSize.at(0).toInt(), viewportSize.at(1).toInt()));

    d->rootViewportElement->load(obj.value("viewport").toObject());

    return true;
}

QMimeData* Timeline::selectedMimeData()
{
    if (d->currentSelection.isEmpty()) return nullptr;

    QMimeData* mimeData = new QMimeData();
    if (d->currentSelection.first()->metaObject()->inherits(&Element::staticMetaObject)) {
        QJsonArray list;

        for (QObject* element : d->currentSelection) {
            //Make sure we're not copying the root viewport
            if (element == d->rootViewportElement) continue;

            //TODO: check that we're not copying any children

            Element* e = static_cast<Element*>(element);
            list.append(e->save());
        }

        mimeData->setData("application/x-theframe-elements", QJsonDocument(list).toBinaryData());
    } else if (d->currentSelection.first()->metaObject()->inherits(&TimelineElement::staticMetaObject)) {
        QJsonArray list;
        for (QObject* element : d->currentSelection) {
            TimelineElement* e = static_cast<TimelineElement*>(element);
            list.append(e->save());
        }
        mimeData->setData("application/x-theframe-timelineelements", QJsonDocument(list).toBinaryData());
    }

    return mimeData;
}

bool Timeline::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == ui->timelineRightPane->viewport()) {
        if (event->type() == QEvent::Gesture) {
            QGestureEvent* e = static_cast<QGestureEvent*>(event);
            if (QGesture* gesture = e->gesture(Qt::PinchGesture)) {
                QPinchGesture* pinchGesture = static_cast<QPinchGesture*>(gesture);
                this->setFrameSpacing(this->frameSpacing() * pinchGesture->scaleFactor(), pinchGesture->centerPoint().x());
            }
        } else if (event->type() == QEvent::Wheel) {
            QWheelEvent* e = static_cast<QWheelEvent*>(event);

#ifdef Q_OS_MAC
            Q_UNUSED(e);
#else
            if (e->modifiers() == Qt::ControlModifier) {
                QPoint pixels = e->pixelDelta();
                QPoint angle = e->angleDelta() / 8;

                if (!pixels.isNull()) {
                    this->setFrameSpacing(this->frameSpacing() * pow(1.005, e->pixelDelta().y()), e->pos().x());
                } else if (!angle.isNull()) {
                    this->setFrameSpacing(this->frameSpacing() * pow(1.005, e->pixelDelta().y()), e->pos().x());
                }

                //Block this event
                return true;
            }
#endif
        }
    }
    return false;
}
