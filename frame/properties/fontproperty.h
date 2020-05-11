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
#ifndef FONTPROPERTY_H
#define FONTPROPERTY_H

#include "propertywidget.h"

namespace Ui {
    class FontProperty;
}

class FontProperty : public PropertyWidget {
        Q_OBJECT

    public:
        Q_INVOKABLE explicit FontProperty(QWidget* parent = nullptr);
        ~FontProperty();

        void setValue(QVariant value);

    private slots:
        void on_fontBox_currentFontChanged(const QFont& f);

        void on_boldButton_toggled(bool checked);

        void on_italiciseButton_toggled(bool checked);

        void on_underlineButton_toggled(bool checked);

    private:
        Ui::FontProperty* ui;

        void setValue();
};

#endif // FONTPROPERTY_H
