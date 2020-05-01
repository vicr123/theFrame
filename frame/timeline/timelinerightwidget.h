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
#ifndef TIMELINERIGHTWIDGET_H
#define TIMELINERIGHTWIDGET_H

#include <QWidget>

namespace Ui {
    class TimelineRightWidget;
}

struct TimelineRightWidgetPrivate;
class Element;
class Timeline;
class TimelineRightWidget : public QWidget {
        Q_OBJECT

    public:
        explicit TimelineRightWidget(Timeline* timeline, Element* element, bool isRoot, QWidget* parent = nullptr);
        ~TimelineRightWidget();

        void addChild(int index, TimelineRightWidget* child);

        void setHeaderHeight(int height);
        void setPropertyElementHeight(int height);
        void setBottomPadding(int padding);

        void setPropertiesCollapsed(bool propertiesCollapsed);

    private:
        Ui::TimelineRightWidget* ui;
        TimelineRightWidgetPrivate* d;

        bool eventFilter(QObject* watched, QEvent* event);

        void paintMainWidget();
};

#endif // TIMELINERIGHTWIDGET_H
