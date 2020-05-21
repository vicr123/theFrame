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
#ifndef TIMELINELEFTWIDGET_H
#define TIMELINELEFTWIDGET_H

#include <QWidget>

namespace Ui {
    class TimelineLeftWidget;
}

class Element;
class TimelineRightWidget;
class Timeline;
struct TimelineLeftWidgetPrivate;
class TimelineLeftWidget : public QWidget {
        Q_OBJECT

    public:
        explicit TimelineLeftWidget(Timeline* timeline, Element* element, bool isRoot, QWidget* parent = nullptr);
        ~TimelineLeftWidget();

        TimelineRightWidget* rightWidget();

    private slots:
        void on_deleteButton_clicked();

        void on_collapseButton_toggled(bool checked);

        void on_renameButton_clicked();

    private:
        Ui::TimelineLeftWidget* ui;
        TimelineLeftWidgetPrivate* d;

        QColor expectedDisplayColor();
        void updatePalette();

        void resizeEvent(QResizeEvent* event);
        void paintEvent(QPaintEvent* event);

        void mousePressEvent(QMouseEvent* event);
        void mouseReleaseEvent(QMouseEvent* event);

        void changeEvent(QEvent* event);
        void contextMenuEvent(QContextMenuEvent *event);

        void addChild(int index, Element* element);
        void addElement(Element* element);
};

#endif // TIMELINELEFTWIDGET_H
