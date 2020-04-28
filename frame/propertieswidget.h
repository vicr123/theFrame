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
#ifndef PROPERTIESWIDGET_H
#define PROPERTIESWIDGET_H

#include <QWidget>

namespace Ui {
    class PropertiesWidget;
}

class Timeline;
class QUndoStack;
struct PropertiesWidgetPrivate;
class PropertiesWidget : public QWidget {
        Q_OBJECT

    public:
        explicit PropertiesWidget(QWidget* parent = nullptr);
        ~PropertiesWidget();

        void setTimeline(Timeline* timeline);
        void setUndoStack(QUndoStack* undoStack);

        void setProjectPath(QString path);

    private slots:
        void on_elementNameBox_textChanged(const QString& arg1);

        void on_easingBox_currentIndexChanged(int index);

        void on_easingTypeBox_currentIndexChanged(int index);

        void on_fpsBox_valueChanged(int arg1);

        void on_resXBox_valueChanged(int arg1);

        void on_resYBox_valueChanged(int arg1);

        void on_totalFramesBox_valueChanged(int arg1);

    private:
        Ui::PropertiesWidget* ui;
        PropertiesWidgetPrivate* d;

        void updateCurrentTimelineElements();
        void setEasingCurve();
};

#endif // PROPERTIESWIDGET_H
