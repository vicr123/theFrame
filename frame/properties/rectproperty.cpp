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
#include "rectproperty.h"
#include "ui_rectproperty.h"

RectProperty::RectProperty(QWidget* parent) :
    PropertyWidget(parent),
    ui(new Ui::RectProperty) {
    ui->setupUi(this);
}

RectProperty::~RectProperty() {
    delete ui;
}

void RectProperty::setValue(QVariant value) {
    PropertyWidget::setValue(value);

    QRect rect = value.toRect();
    ui->leftBox->setValue(rect.left());
    ui->topBox->setValue(rect.top());
    ui->rightBox->setValue(rect.right() + 1);
    ui->bottomBox->setValue(rect.bottom() + 1);
    ui->widthBox->setValue(rect.width());
    ui->heightBox->setValue(rect.height());
}

void RectProperty::on_leftBox_valueChanged(int arg1) {
    QRect rect = this->value().toRect();
    rect.setLeft(arg1);
    this->setValue(rect);
}

void RectProperty::on_topBox_valueChanged(int arg1) {
    QRect rect = this->value().toRect();
    rect.setTop(arg1);
    this->setValue(rect);
}

void RectProperty::on_bottomBox_valueChanged(int arg1) {
    QRect rect = this->value().toRect();
    rect.setBottom(arg1 - 1);
    this->setValue(rect);
}

void RectProperty::on_rightBox_valueChanged(int arg1) {
    QRect rect = this->value().toRect();
    rect.setRight(arg1 - 1);
    this->setValue(rect);
}

void RectProperty::on_widthBox_valueChanged(int arg1) {
    QRect rect = this->value().toRect();
    rect.setWidth(arg1);
    this->setValue(rect);
}

void RectProperty::on_heightBox_valueChanged(int arg1) {
    QRect rect = this->value().toRect();
    rect.setHeight(arg1);
    this->setValue(rect);
}
