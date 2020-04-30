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
#ifndef FILEPROPERTY_H
#define FILEPROPERTY_H

#include "propertywidget.h"

namespace Ui {
    class FileProperty;
}

struct FilePropertyPrivate;
class FileProperty : public PropertyWidget {
        Q_OBJECT

    public:
        Q_INVOKABLE explicit FileProperty(QWidget* parent = nullptr);
        ~FileProperty();

        void setValue(QVariant value);
        void setPropertyMetadata(QVariantMap metadata);

    private slots:
        void on_browseButton_clicked();

        void on_filenameBox_textEdited(const QString& arg1);

    private:
        Ui::FileProperty* ui;
        FilePropertyPrivate* d;

};

#endif // FILEPROPERTY_H
