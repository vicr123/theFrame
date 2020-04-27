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
#include "stringproperty.h"
#include "ui_stringproperty.h"

StringProperty::StringProperty(QWidget* parent) :
    PropertyWidget(parent),
    ui(new Ui::StringProperty) {
    ui->setupUi(this);
}

StringProperty::~StringProperty() {
    delete ui;
}

void StringProperty::setValue(QVariant value) {
    PropertyWidget::setValue(value);
    ui->valueBox->setText(value.toString());
}

void StringProperty::on_valueBox_textChanged(const QString& arg1) {
    this->setValue(arg1);
}
