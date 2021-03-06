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
#include "fileproperty.h"
#include "ui_fileproperty.h"

#include <QFileDialog>

struct FilePropertyPrivate {
    QStringList namefilters;
};

FileProperty::FileProperty(QWidget* parent) :
    PropertyWidget(parent),
    ui(new Ui::FileProperty) {
    ui->setupUi(this);
    d = new FilePropertyPrivate();
    d->namefilters = QStringList({tr("All Files (*)")});
}

FileProperty::~FileProperty() {
    delete ui;
    delete d;
}

void FileProperty::setValue(QVariant value) {
    PropertyWidget::setValue(value);

    ui->filenameBox->setText(value.toString());
}

void FileProperty::on_browseButton_clicked() {
    QFileDialog* fileDialog = new QFileDialog(this->window());
    fileDialog->setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog->setNameFilters(d->namefilters);
    fileDialog->setWindowFlag(Qt::Sheet);
    fileDialog->setWindowModality(Qt::WindowModal);
    connect(fileDialog, &QFileDialog::finished, this, [ = ](int result) {
        if (result == QFileDialog::Accepted) {
            //Attempt to load this file
            QDir dir(this->property("projectPath").toString());
            this->setValue(dir.relativeFilePath(fileDialog->selectedFiles().first()));
        }
        fileDialog->deleteLater();
    });
    fileDialog->open();
}

void FileProperty::on_filenameBox_textEdited(const QString& arg1) {
    this->setValue(arg1);
}


void FileProperty::setPropertyMetadata(QVariantMap metadata)
{
    d->namefilters = metadata.value("nameFilters", QStringList({tr("All Files (*)")})).toStringList();
}
