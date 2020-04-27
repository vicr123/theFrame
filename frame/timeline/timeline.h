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
#ifndef TIMELINE_H
#define TIMELINE_H

#include <QWidget>
#include <elements/viewportelement.h>

namespace Ui {
    class Timeline;
}

struct TimelinePrivate;
class TimelineElement;
class Prerenderer;
class Timeline : public QWidget {
        Q_OBJECT

    public:
        explicit Timeline(QWidget* parent = nullptr);
        ~Timeline();

        void setViewportElement(ViewportElement* element);
        ViewportElement* viewportElement();

        void setPrerenderer(Prerenderer* prerenderer);
        Prerenderer* prerenderer() const;

        void setFrameSpacing(double frameSpacing);
        double frameSpacing() const;

        void setFrameCount(quint64 frameCount);
        quint64 frameCount() const;

        void setFramerate(uint framerate);
        uint framerate() const;

        void setCurrentFrame(quint64 frame);
        quint64 currentFrame() const;

        void setCurrentSelection(QObject* element);
        void addToCurrentSelection(QObject* element);
        void clearCurrentSelection();
        QList<QObject*> currentSelection();

        void deleteSelected();

        QJsonObject save() const;
        bool load(QJsonObject obj);

    signals:
        void frameSpacingChanged(double frameSpacing);
        void frameCountChanged(quint64 frameCount);
        void framerateChanged(uint framerate);
        void currentFrameChanged(quint64 currentFrame);
        void currentSelectionChanged();

    private:
        Ui::Timeline* ui;
        TimelinePrivate* d;
};

#endif // TIMELINE_H
