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
#include "pointproperty.h"
#include "ui_pointproperty.h"

PointProperty::PointProperty(QWidget* parent) :
    PropertyWidget(parent),
    ui(new Ui::PointProperty) {
    ui->setupUi(this);
}

PointProperty::~PointProperty() {
    delete ui;
}

void PointProperty::setValue(QVariant value) {
    PropertyWidget::setValue(value);

    QPoint point = value.toPoint();
    ui->xBox->setValue(point.x());
    ui->yBox->setValue(point.y());
}

void PointProperty::on_xBox_valueChanged(int arg1) {
    QPoint point = this->value().toPoint();
    point.setX(arg1);
    this->setValue(point);
}

void PointProperty::on_yBox_valueChanged(int arg1) {
    QPoint point = this->value().toPoint();
    point.setY(arg1);
    this->setValue(point);
}
