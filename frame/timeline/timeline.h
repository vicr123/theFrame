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

class QMimeData;
class TimelineElement;
class Prerenderer;
class QUndoStack;
class TimelineRightWidgetProperty;
class TutorialEngine;
struct TimelinePrivate;
class Timeline : public QWidget {
        Q_OBJECT

    public:
        enum LoadError {
            NoError,
            FileVersionTooNew,
            FileCorrupt
        };

        explicit Timeline(QWidget* parent = nullptr);
        ~Timeline();

        void setUndoStack(QUndoStack* undoStack);
        QUndoStack* undoStack();

        void setViewportElement(ViewportElement* element);
        ViewportElement* viewportElement();

        void setPrerenderer(Prerenderer* prerenderer);
        Prerenderer* prerenderer() const;

        void setTutorialEngine(TutorialEngine* engine);
        TutorialEngine* tutorialEngine();

        void setFrameSpacing(double frameSpacing, int centerX = 0);
        double frameSpacing() const;

        void setFrameCount(quint64 frameCount);
        quint64 frameCount() const;

        quint64 leftRenderFrame();
        quint64 rightRenderFrame();
        void ensurePlayheadVisible();
        int playheadXPos();

        void setFramerate(uint framerate);
        uint framerate() const;

        void setCurrentFrame(quint64 frame);
        quint64 currentFrame() const;

        void setCurrentSelection(QObject* element);
        void addToCurrentSelection(QObject* element);
        void clearCurrentSelection();
        QList<QObject*> currentSelection();

        void setSelectedTimelineRightWidget(TimelineRightWidgetProperty* rightWidget);
        TimelineRightWidgetProperty* selectedTimelineRightWidget();

        void deleteSelected(QString undoText = "");

        void setInPoint(quint64 inPoint);
        quint64 inPoint();

        void setOutPoint(quint64 outPoint);
        quint64 outPoint();

        void clearInOutPoint();
        bool isInPreviewRange(quint64 frame);

        bool canCopy();
        void copy();

        bool canCut();
        void cut();

        bool canPaste();
        void paste();

        void selectAll();

        QJsonObject save() const;
        LoadError load(QJsonObject obj);

    signals:
        void frameSpacingChanged(double frameSpacing);
        void frameCountChanged(quint64 frameCount);
        void framerateChanged(uint framerate);
        void currentFrameChanged(quint64 currentFrame);
        void inOutPointChanged(quint64 inPoint, quint64 outPoint);
        void currentSelectionChanged();

        void canCopyChanged();
        void canCutChanged();
        void canPasteChanged();

    private:
        Ui::Timeline* ui;
        TimelinePrivate* d;

        QMimeData* selectedMimeData();
        bool eventFilter(QObject* watched, QEvent* event);
};

#endif // TIMELINE_H
