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
#ifndef PRERENDERER_H
#define PRERENDERER_H

#include <QObject>
#include <tpromise.h>

class ViewportElement;
class Timeline;
struct PrerenderPrivate;
class Prerenderer : public QObject {
        Q_OBJECT
    public:
        explicit Prerenderer(QObject* parent = nullptr);
        ~Prerenderer();

        void setViewportElement(ViewportElement* element);
        void setTimeline(Timeline* timeline);

        void setEnablePrerendering(bool enable);

        tPromise<QPixmap>* frame(quint64 frame);
        tPromise<QPixmap>* renderFrame(quint64 frame);

        bool isFrameCached(quint64 frame);

        void tryPrerenderAll();

    signals:
        void framePrerenderStateChanged(quint64 frame);
        void prerenderInvalidated();

    private:
        PrerenderPrivate* d;

        tPromise<void>* cacheFrame(quint64 frame);

        quint64 nextPrerenderFrame(quint64 frame);
        void preprerenderNextFrame(quint64 frame);
        void prerenderNextFrame(quint64 frame);
        void completePrerenderJob();
};

#endif // PRERENDERER_H
