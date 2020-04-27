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
#ifndef FLOATPROPERTY_H
#define FLOATPROPERTY_H

#include "propertywidget.h"

namespace Ui {
    class FloatProperty;
}

class FloatProperty : public PropertyWidget {
        Q_OBJECT

    public:
        Q_INVOKABLE explicit FloatProperty(QWidget* parent = nullptr);
        ~FloatProperty();

        void setValue(QVariant value);

    private slots:
        void on_propertyBox_valueChanged(double arg1);

    private:
        Ui::FloatProperty* ui;
};

#endif // FLOATPROPERTY_H
