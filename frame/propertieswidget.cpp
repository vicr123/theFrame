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
#include "propertieswidget.h"
#include "ui_propertieswidget.h"

#include "properties/rectproperty.h"
#include "properties/floatproperty.h"
#include "properties/percentageproperty.h"
#include "properties/stringproperty.h"
#include "properties/pointproperty.h"
#include "properties/fontproperty.h"
#include "properties/fileproperty.h"

#include "timeline/timeline.h"
#include <elements/timelineelement.h>
#include <elements/element.h>

#include "undo/undoelementmodify.h"
#include "undo/undotimelineelementmodify.h"

struct PropertiesWidgetPrivate {
    Timeline* timeline;
    QUndoStack* undoStack;

    QString projectPath;

    PropertyWidget* startWidget = nullptr;
    PropertyWidget* endWidget = nullptr;

    QList<QWidget*> propertyWidgets;

    const QMetaObject* objectForPropertyType(Element::PropertyType type) {
        switch (type) {
            case Element::Percentage:
                return &PercentageProperty::staticMetaObject;
            case Element::Integer:
                break;
            case Element::Double:
                return &FloatProperty::staticMetaObject;
            case Element::Rect:
                return &RectProperty::staticMetaObject;
            case Element::Color:
                break;
            case Element::String:
                return &StringProperty::staticMetaObject;
            case Element::Point:
                return &PointProperty::staticMetaObject;
            case Element::Font:
                return &FontProperty::staticMetaObject;
            case Element::File:
                return &FileProperty::staticMetaObject;

        }
        return nullptr;
    }
};

PropertiesWidget::PropertiesWidget(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::PropertiesWidget) {
    ui->setupUi(this);
    d = new PropertiesWidgetPrivate();
}

PropertiesWidget::~PropertiesWidget() {
    delete d;
    delete ui;
}

void PropertiesWidget::setTimeline(Timeline* timeline) {
    QSignalBlocker blocker(ui->fpsBox);
    QSignalBlocker blocker2(ui->totalFramesBox);
    QSignalBlocker blocker3(ui->resXBox);
    QSignalBlocker blocker4(ui->resYBox);
    d->timeline = timeline;
    connect(timeline, &Timeline::currentSelectionChanged, this, &PropertiesWidget::updateCurrentTimelineElements);
    connect(timeline, &Timeline::framerateChanged, ui->fpsBox, &QSpinBox::setValue);
    connect(timeline, &Timeline::frameCountChanged, ui->totalFramesBox, &QSpinBox::setValue);
    connect(timeline->viewportElement(), &ViewportElement::viewportSizeChanged, this, [ = ](QSize size) {
        ui->resXBox->setValue(size.width());
        ui->resYBox->setValue(size.height());
    });

    ui->fpsBox->setValue(static_cast<int>(timeline->framerate()));
    ui->totalFramesBox->setValue(static_cast<int>(timeline->frameCount()));
    ui->resXBox->setValue(timeline->viewportElement()->viewportSize().width());
    ui->resYBox->setValue(timeline->viewportElement()->viewportSize().height());
}

void PropertiesWidget::setUndoStack(QUndoStack* undoStack)
{
    d->undoStack = undoStack;
}

void PropertiesWidget::setProjectPath(QString path) {
    d->projectPath = path;
}

void PropertiesWidget::updateCurrentTimelineElements() {
    if (d->timeline->currentSelection().count() == 0) {
        ui->stackedWidget->setCurrentWidget(ui->nothingSelectedPage);
    } else if (d->timeline->currentSelection().count() == 1) {
        QObject* e = d->timeline->currentSelection().first();

        if (e->metaObject()->inherits(&TimelineElement::staticMetaObject)) {
            ui->stackedWidget->setCurrentWidget(ui->singleTimelineElementSelectedPage);

            TimelineElement* timelineElement = qobject_cast<TimelineElement*>(e);
            connect(timelineElement, &TimelineElement::aboutToDelete, this, &PropertiesWidget::updateCurrentTimelineElements);
            if (d->startWidget) {
                ui->startValueLayout->removeWidget(d->startWidget);
                d->startWidget->deleteLater();
                d->startWidget = nullptr;
            }
            if (d->endWidget) {
                ui->endValueLayout->removeWidget(d->endWidget);
                d->endWidget->deleteLater();
                d->endWidget = nullptr;
            }

            const QMetaObject* metaObj = d->objectForPropertyType(timelineElement->parentElement()->propertyType(timelineElement->propertyName()));
            if (metaObj) {
                d->startWidget = qobject_cast<PropertyWidget*>(metaObj->newInstance());
                d->endWidget = qobject_cast<PropertyWidget*>(metaObj->newInstance());

                d->startWidget->setProperty("projectPath", d->projectPath);
                d->endWidget->setProperty("projectPath", d->projectPath);

                ui->startValueLayout->addWidget(d->startWidget);
                ui->endValueLayout->addWidget(d->endWidget);
                d->startWidget->setValue(timelineElement->startValue());
                d->endWidget->setValue(timelineElement->endValue());

                connect(d->startWidget, &PropertyWidget::valueChanged, this, [=](QVariant value) {
                    TimelineElementState oldState(timelineElement);
                    timelineElement->setStartValue(value);
                    d->undoStack->push(new UndoTimelineElementModify(tr("Start Value Change"), oldState, TimelineElementState(timelineElement)));
                });
                connect(d->endWidget, &PropertyWidget::valueChanged, this, [=](QVariant value) {
                    TimelineElementState oldState(timelineElement);
                    timelineElement->setEndValue(value);
                    d->undoStack->push(new UndoTimelineElementModify(tr("End Value Change"), oldState, TimelineElementState(timelineElement)));
                });
                connect(timelineElement, &TimelineElement::elementPropertyChanged, d->startWidget, [=] {
                    QSignalBlocker blocker(d->startWidget);
                    d->startWidget->setValue(timelineElement->startValue());
                });
                connect(timelineElement, &TimelineElement::elementPropertyChanged, d->endWidget, [=] {
                    QSignalBlocker blocker(d->endWidget);
                    d->endWidget->setValue(timelineElement->endValue());
                });
            }

            if (timelineElement->easingCurve().type() == QEasingCurve::Linear) {
                ui->easingBox->setCurrentIndex(0);
            } else if (timelineElement->easingCurve().type() == QEasingCurve::BezierSpline) {
                ui->easingBox->setCurrentIndex(ui->easingBox->count() - 1);
            } else {
                ui->easingBox->setCurrentIndex(1 + timelineElement->easingCurve().type() / 4);
            }
        } else if (e->metaObject()->inherits(&Element::staticMetaObject)) {
            ui->stackedWidget->setCurrentWidget(ui->singleElementSelectedPage);
            for (QWidget* w : d->propertyWidgets) {
                ui->elementPropertiesLayout->removeWidget(w);
                w->deleteLater();
            }
            d->propertyWidgets.clear();

            Element* element = qobject_cast<Element*>(e);
            for (QString property : element->allProperties().keys()) {
                QWidget* w = new QWidget(this);

                QBoxLayout* outerLayout = new QBoxLayout(QBoxLayout::TopToBottom);
                outerLayout->setContentsMargins(0, 0, 0, 0);
                w->setLayout(outerLayout);

                QBoxLayout* layout = new QBoxLayout(QBoxLayout::TopToBottom);
                layout->setContentsMargins(9, 9, 9, 9);
                outerLayout->addLayout(layout);

                QLabel* titleLabel = new QLabel(w);
                QFont labelFont = titleLabel->font();
                labelFont.setBold(true);
                titleLabel->setFont(labelFont);
                titleLabel->setText(element->propertyDisplayName(property).toUpper());
                layout->addWidget(titleLabel);

                const QMetaObject* metaObj = d->objectForPropertyType(element->propertyType(property));
                if (metaObj) {
                    PropertyWidget* pw = qobject_cast<PropertyWidget*>(metaObj->newInstance(Q_ARG(QWidget*, w)));
                    pw->setProperty("projectPath", d->projectPath);
                    pw->setValue(element->startValue(property));
                    connect(pw, &PropertyWidget::valueChanged, this, [ = ](QVariant value) {
                        ElementState oldState(element);
                        element->setStartValue(property, value);
                        d->undoStack->push(new UndoElementModify(tr("Start State Change"), oldState, ElementState(element)));
                    });
                    layout->addWidget(pw);
                }

                QFrame* line = new QFrame(w);
                line->setFrameShape(QFrame::HLine);
                line->setFixedHeight(1);
                outerLayout->addWidget(line);

                ui->elementPropertiesLayout->addWidget(w);
                d->propertyWidgets.append(w);
            }

            ui->elementType->setText(element->typeDisplayName().toUpper());
            ui->elementNameBox->setText(element->name());
        }
    }
}

void PropertiesWidget::setEasingCurve() {
    TimelineElement* timelineElement = qobject_cast<TimelineElement*>(d->timeline->currentSelection().first());
    TimelineElementState oldState(timelineElement);
    if (ui->easingBox->currentIndex() == 0) { //Linear
        timelineElement->setEasingCurve(QEasingCurve::Linear);
    } else if (ui->easingBox->currentIndex() == ui->easingBox->count() - 1) {
        timelineElement->setEasingCurve(QEasingCurve::BezierSpline);
    } else {
        QEasingCurve::Type easingCurveType = static_cast<QEasingCurve::Type>((ui->easingBox->currentIndex() - 1) * 4 + 1 + ui->easingTypeBox->currentIndex());
        timelineElement->setEasingCurve(easingCurveType);
    }
    d->undoStack->push(new UndoTimelineElementModify(tr("Easing Curve Change"), oldState, TimelineElementState(timelineElement)));
}

void PropertiesWidget::on_elementNameBox_textChanged(const QString& arg1) {
    Element* element = qobject_cast<Element*>(d->timeline->currentSelection().first());
    ElementState oldState(element);
    element->setName(arg1);
    d->undoStack->push(new UndoElementModify(tr("Element Name Change"), oldState, ElementState(element)));
}

void PropertiesWidget::on_easingBox_currentIndexChanged(int index) {
    if (index == 0 || index == ui->easingBox->count() - 1) {
        ui->easingTypeBox->setEnabled(false);
    } else {
        ui->easingTypeBox->setEnabled(true);
    }
    setEasingCurve();
}

void PropertiesWidget::on_easingTypeBox_currentIndexChanged(int index) {
    setEasingCurve();
}

void PropertiesWidget::on_fpsBox_valueChanged(int arg1) {
    d->timeline->setFramerate(static_cast<uint>(arg1));
}

void PropertiesWidget::on_resXBox_valueChanged(int arg1) {
    d->timeline->viewportElement()->setViewportSize(QSize(arg1, ui->resYBox->value()));
}

void PropertiesWidget::on_resYBox_valueChanged(int arg1) {
    d->timeline->viewportElement()->setViewportSize(QSize(ui->resXBox->value(), arg1));
}

void PropertiesWidget::on_totalFramesBox_valueChanged(int arg1) {
    d->timeline->setFrameCount(static_cast<uint>(arg1));
}
