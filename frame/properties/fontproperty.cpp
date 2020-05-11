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
#include "fontproperty.h"
#include "ui_fontproperty.h"

FontProperty::FontProperty(QWidget* parent) :
    PropertyWidget(parent),
    ui(new Ui::FontProperty) {
    ui->setupUi(this);
}

FontProperty::~FontProperty() {
    delete ui;
}

void FontProperty::setValue(QVariant value) {
    PropertyWidget::setValue(value);

    QFont font = value.value<QFont>();
    ui->fontBox->setCurrentFont(font);
}

void FontProperty::on_fontBox_currentFontChanged(const QFont& f) {
    this->setValue();
}

void FontProperty::setValue() {
    QFont font(ui->fontBox->currentFont().family());

    this->setValue(font);
}
