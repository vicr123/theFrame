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
#ifndef TIMELINERIGHTWIDGETPROPERTY_H
#define TIMELINERIGHTWIDGETPROPERTY_H

#include <QWidget>

struct TimelineRightWidgetPropertyPrivate;
class Element;
class Timeline;
class TimelineRightWidgetProperty : public QWidget {
        Q_OBJECT
    public:
        explicit TimelineRightWidgetProperty(Timeline* timeline, Element* element, QString property, bool isRoot, QWidget* parent = nullptr);
        ~TimelineRightWidgetProperty();

        quint64 frameForPoint(int x);

    signals:

    private:
        TimelineRightWidgetPropertyPrivate* d;

        void paintEvent(QPaintEvent* event);
        void mousePressEvent(QMouseEvent* event);
        void mouseMoveEvent(QMouseEvent* event);
        void mouseReleaseEvent(QMouseEvent* event);
        void focusOutEvent(QFocusEvent* event);

        void timerParametersChanged();
        void draw();
};

#endif // TIMELINERIGHTWIDGETPROPERTY_H
