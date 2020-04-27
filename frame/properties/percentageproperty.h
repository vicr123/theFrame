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
#ifndef PERCENTAGEPROPERTY_H
#define PERCENTAGEPROPERTY_H

#include "propertywidget.h"

namespace Ui {
    class PercentageProperty;
}

class PercentageProperty : public PropertyWidget {
        Q_OBJECT

    public:
        Q_INVOKABLE explicit PercentageProperty(QWidget* parent = nullptr);
        ~PercentageProperty();

        void setValue(QVariant value);

    private slots:
        void on_valueBox_valueChanged(int arg1);

    private:
        Ui::PercentageProperty* ui;
};

#endif // PERCENTAGEPROPERTY_H
