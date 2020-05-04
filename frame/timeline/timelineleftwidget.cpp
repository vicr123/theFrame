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
#include "timelineleftwidget.h"
#include "ui_timelineleftwidget.h"

#include "timeline.h"
#include "timelinerightwidget.h"
#include <QPainter>
#include <QMenu>
#include <QInputDialog>
#include <QMouseEvent>
#include <QPointer>

#include <QUndoStack>
#include "undo/undonewelement.h"
#include "undo/undodeleteelement.h"
#include "undo/undoelementmodify.h"

#include <elements/rectangleelement.h>
#include <elements/textelement.h>
#include <elements/groupelement.h>
#include <elements/pictureelement.h>
#include <elements/ellipseelement.h>

struct TimelineLeftWidgetPrivate {
    Timeline* timeline;
    QPointer<Element> element;
    TimelineRightWidget* rightWidget;

    bool isRoot;
};

TimelineLeftWidget::TimelineLeftWidget(Timeline* timeline, Element* element, bool isRoot, QWidget* parent) :
    QWidget(parent),
    ui(new Ui::TimelineLeftWidget) {
    ui->setupUi(this);

    d = new TimelineLeftWidgetPrivate();
    d->timeline = timeline;
    d->element = element;
    d->isRoot = isRoot;

    d->rightWidget = new TimelineRightWidget(timeline, element, isRoot);

    connect(element, &Element::nameChanged, ui->elementName, &QLabel::setText);
    ui->elementName->setText(element->name());
    ui->elementType->setText(element->typeDisplayName());

    int propertyElementHeight = 0;
    for (QString property : element->animatableProperties().keys()) {
        QLabel* l = new QLabel();
        QFont labelFont = l->font();
        labelFont.setPointSizeF(labelFont.pointSizeF() * 0.8);
        l->setFont(labelFont);
        l->setText(element->propertyDisplayName(property));
        l->setMargin(3);
        l->setAutoFillBackground(true);

        QPalette pal = l->palette();
        pal.setColor(QPalette::Window, element->propertyColor(property));
        l->setPalette(pal);

        ui->propertiesLayout->addWidget(l);

        propertyElementHeight = l->sizeHint().height();
    }
    d->rightWidget->setPropertyElementHeight(propertyElementHeight);
    d->rightWidget->setHeaderHeight(ui->headerWidget->sizeHint().height());
    d->rightWidget->setBottomPadding(ui->addButtonsContainer->sizeHint().height());

    for (int i = 0; i < element->childElements().count(); i++) {
        Element* child = element->childElements().at(i);
        addChild(i, child);
    }
    connect(element, &Element::childElementInserted, this, &TimelineLeftWidget::addChild);
    connect(element, &Element::destroyed, this, &TimelineLeftWidget::deleteLater);

    connect(timeline, &Timeline::currentSelectionChanged, this, [ = ] {
        this->update();
    });

    QMenu* addMenu = new QMenu(this);
    addMenu->addSection(tr("Add an element"));
    addMenu->addAction(tr("Rectangle"), [ = ] {
        addElement(new RectangleElement());
    });
    addMenu->addAction(tr("Ellipse"), [ = ] {
        addElement(new EllipseElement());
    });
    addMenu->addAction(tr("Text"), [ = ] {
        addElement(new TextElement());
    });
    addMenu->addAction(tr("Picture"), [ = ] {
        addElement(new PictureElement());
    });
    addMenu->addAction(tr("Group"), [ = ] {
        addElement(new GroupElement());
    });
    ui->addButton->setMenu(addMenu);

    if (isRoot) ui->deleteButton->setVisible(false);
}

TimelineLeftWidget::~TimelineLeftWidget() {
    delete d;
    delete ui;
}

TimelineRightWidget* TimelineLeftWidget::rightWidget() {
    return d->rightWidget;
}

void TimelineLeftWidget::resizeEvent(QResizeEvent* event) {
    d->rightWidget->setFixedHeight(this->height());
}

void TimelineLeftWidget::paintEvent(QPaintEvent* event) {
    if (!d->element) return;

    QPainter painter(this);
    painter.setPen(Qt::transparent);

    QColor displayColor = d->element->displayColor();
    if (d->isRoot) displayColor.setAlpha(255);
    if (d->timeline->currentSelection().contains(d->element)) displayColor = displayColor.lighter();
    painter.setBrush(displayColor);
    painter.drawRect(0, 0, this->width(), this->height());
}

void TimelineLeftWidget::mousePressEvent(QMouseEvent* event) {
    if ((event->modifiers() & Qt::ControlModifier) == 0) {
        d->timeline->clearCurrentSelection();
    }
    d->timeline->addToCurrentSelection(d->element);
}

void TimelineLeftWidget::mouseReleaseEvent(QMouseEvent* event) {

}

void TimelineLeftWidget::addChild(int index, Element* element) {
    TimelineLeftWidget* widget = new TimelineLeftWidget(d->timeline, element, false);
    d->rightWidget->addChild(index, widget->rightWidget());
    ui->childrenLayout->insertWidget(index, widget);
}

void TimelineLeftWidget::addElement(Element* element)
{
    d->element->addChild(element);
    d->timeline->undoStack()->push(new UndoNewElement(tr("Add %1").arg(element->typeDisplayName()), ElementState(element)));
}

void TimelineLeftWidget::on_deleteButton_clicked() {
    if (d->element->childElements().count() > 0) {
        QMenu* warningMenu = new QMenu(this);
        warningMenu->addSection(tr("Delete"));
        warningMenu->addAction(tr("Children will also be deleted"))->setEnabled(false);
        warningMenu->addAction(QIcon::fromTheme("list-remove"), tr("Delete"), [ = ] {
            d->timeline->undoStack()->push(new UndoDeleteElement(tr("Delete %1 \"%2\"").arg(d->element->typeDisplayName()).arg(d->element->name()), ElementState(d->element)));
            d->element->deleteLater();
        });
        ui->deleteButton->setMenu(warningMenu);
        ui->deleteButton->showMenu();
    } else {
        d->timeline->undoStack()->push(new UndoDeleteElement(tr("Delete %1 \"%2\"").arg(d->element->typeDisplayName()).arg(d->element->name()), ElementState(d->element)));
        d->element->deleteLater();
    }
}

void TimelineLeftWidget::on_collapseButton_toggled(bool checked) {
    d->rightWidget->setPropertiesCollapsed(checked);
    ui->propertiesWidget->setVisible(!checked);
    ui->addButtonsContainer->setVisible(!checked);
}

void TimelineLeftWidget::on_renameButton_clicked() {
    bool ok;
    QString newName = QInputDialog::getText(this, tr("Rename"), tr("Set a name for this %1").arg(d->element->typeDisplayName()), QLineEdit::Normal, d->element->name(), &ok);
    if (ok) {
        ElementState oldState(d->element);
        d->element->setName(newName);
        d->timeline->undoStack()->push(new UndoElementModify(tr("Element Name Change"), oldState, ElementState(d->element)));
    }
}
