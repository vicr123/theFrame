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
#include "propertywidget.h"

struct PropertyWidgetPrivate {
    QVariant value;
};

PropertyWidget::PropertyWidget(QWidget* parent) : QWidget(parent) {
    d = new PropertyWidgetPrivate();
}

PropertyWidget::~PropertyWidget() {
    delete d;
}

void PropertyWidget::setValue(QVariant value) {
    d->value = value;
    emit valueChanged(value);
}

QVariant PropertyWidget::value() {
    return d->value;
}

void PropertyWidget::setPropertyMetadata(QVariantMap metadata)
{
    Q_UNUSED(metadata);
}
