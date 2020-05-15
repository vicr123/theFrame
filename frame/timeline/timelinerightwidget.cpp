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
#include "timelinerightwidget.h"
#include "ui_timelinerightwidget.h"

#include <elements/element.h>
#include "timelinerightwidgetproperty.h"
#include <QPainter>

#include "timeline.h"
#include "tutorialengine.h"
#include "tutorialwindow.h"

struct TimelineRightWidgetPrivate {
    Timeline* timeline;
    Element* element;
    bool isRoot;

    int bottomPadding;

    TimelineRightWidgetProperty* mainPropertyWidget;
    QList<TimelineRightWidgetProperty*> propertyWidgets;
};

TimelineRightWidget::TimelineRightWidget(Timeline* timeline, Element* element, bool isRoot, QWidget* parent) :
    QWidget(parent),
    ui(new Ui::TimelineRightWidget) {
    ui->setupUi(this);

    d = new TimelineRightWidgetPrivate();
    d->timeline = timeline;
    d->element = element;
    d->isRoot = isRoot;

    d->mainPropertyWidget = new TimelineRightWidgetProperty(timeline, element, "", isRoot, this);
    if (!isRoot) {
        ui->mainWidgetLayout->addWidget(d->mainPropertyWidget);
    }

    for (QString property : element->animatableProperties().keys()) {
        TimelineRightWidgetProperty* propertyWidget = new TimelineRightWidgetProperty(timeline, element, property, false, this);
        ui->propertiesLayout->addWidget(propertyWidget);
        d->propertyWidgets.append(propertyWidget);
    }

    d->timeline->tutorialEngine()->setTutorialTrigger(TutorialEngine::AddTimelineElement, [=] {
        TutorialWindow::trigger(TutorialWindow::AddTimelineElement, TutorialWindow::Vertical, ui->propertiesWidget);
    }, [=] {
        TutorialWindow::hide(TutorialWindow::AddTimelineElement);
    });

    connect(element, &Element::destroyed, this, &TimelineRightWidget::deleteLater);
}

TimelineRightWidget::~TimelineRightWidget() {
    delete d;
    delete ui;
}

bool TimelineRightWidget::isRoot()
{
    return d->isRoot;
}

QWidget* TimelineRightWidget::mainPropertyWidget()
{
    return d->mainPropertyWidget;
}

void TimelineRightWidget::addChild(int index, TimelineRightWidget* child) {
    ui->childrenLayout->insertWidget(index, child);
}

void TimelineRightWidget::setHeaderHeight(int height) {
    ui->mainWidget->setFixedHeight(height);
    d->mainPropertyWidget->setFixedHeight(height);
}

void TimelineRightWidget::setPropertyElementHeight(int height) {
    for (TimelineRightWidgetProperty* widget : d->propertyWidgets) {
        widget->setFixedHeight(height);
    }
}

void TimelineRightWidget::setBottomPadding(int padding) {
    d->bottomPadding = padding;
    ui->childrenLayout->setContentsMargins(0, 0, 0, padding);
}

void TimelineRightWidget::setPropertiesCollapsed(bool propertiesCollapsed) {
    ui->propertiesWidget->setVisible(!propertiesCollapsed);
    if (propertiesCollapsed) {
        ui->childrenLayout->setContentsMargins(0, 0, 0, 0);
    } else {
        ui->childrenLayout->setContentsMargins(0, 0, 0, d->bottomPadding);
    }
}

void TimelineRightWidget::setPalette(QPalette pal)
{
    d->mainPropertyWidget->setPalette(pal);
    QWidget::setPalette(pal);
}

bool TimelineRightWidget::eventFilter(QObject* watched, QEvent* event) {
    if (watched == ui->mainWidget) {
        switch (event->type()) {
            case QEvent::Paint:
                paintMainWidget();
                break;
            default:
                break;
        }
    }
    return false;
}

void TimelineRightWidget::paintMainWidget() {
    QPainter painter(ui->mainWidget);
    painter.setPen(Qt::transparent);
    painter.setBrush(QColor(qrand()));
    painter.drawRect(0, 0, this->width(), this->height());
}
