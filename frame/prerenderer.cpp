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
#include "prerenderer.h"

#include <elements/viewportelement.h>
#include "timeline/timeline.h"

struct PrerenderPrivate {
    ViewportElement* element;
    Timeline* timeline;
    QMap<quint64, QPixmap> prerenders;

    bool prerenderJobRunning = false;
    bool restartPrerenderJob = false;
};

Prerenderer::Prerenderer(QObject* parent) : QObject(parent) {
    d = new PrerenderPrivate();
}

Prerenderer::~Prerenderer() {
    delete d;
}

void Prerenderer::setViewportElement(ViewportElement* element) {
    d->element = element;
    connect(element, &ViewportElement::invalidateFromFrame, this, [ = ](quint64 frame) {
        for (quint64 i = frame; i < d->timeline->frameCount(); i++) {
            if (d->prerenders.contains(i)) {
                d->prerenders.remove(i);
                emit framePrerenderStateChanged(i);
            }
        }
        this->tryPrerenderAll();
    });
}

void Prerenderer::setTimeline(Timeline* timeline) {
    d->timeline = timeline;
}

tPromise<QPixmap>* Prerenderer::frame(quint64 frame) {
    return tPromise<QPixmap>::runOnSameThread([ = ](tPromiseFunctions<QPixmap>::SuccessFunction res, tPromiseFunctions<QPixmap>::FailureFunction rej) {
        if (d->prerenders.contains(frame)) {
            res(d->prerenders.value(frame));
        } else {
            this->cacheFrame(frame)->then([ = ] {
                res(d->prerenders.value(frame));
            });
        }
    });
}

tPromise<void>* Prerenderer::cacheFrame(quint64 frame) {
    return tPromise<void>::runOnSameThread([ = ](tPromiseFunctions<void>::SuccessFunction res, tPromiseFunctions<void>::FailureFunction rej) {
        return tPromise<QPixmap>::runOnNewThread([ = ](tPromiseFunctions<QPixmap>::SuccessFunction res, tPromiseFunctions<QPixmap>::FailureFunction rej) {
            QPixmap pixmap(d->element->viewportSize());
            QPainter painter(&pixmap);
            d->element->render(&painter, frame);
            painter.end();
            pixmap = pixmap.scaled(pixmap.size() / 8);
            res(pixmap);
        })->then([ = ](QPixmap pixmap) {
            d->prerenders.insert(frame, pixmap);
            emit framePrerenderStateChanged(frame);
            res();
        });
    });
}

bool Prerenderer::isFrameCached(quint64 frame) {
    return d->prerenders.contains(frame);
}

void Prerenderer::tryPrerenderAll() {
    if (d->prerenderJobRunning) {
        d->restartPrerenderJob = true;
        return;
    }

    d->prerenderJobRunning = true;
    d->restartPrerenderJob = false;

    preprerenderNextFrame(0);
}

quint64 Prerenderer::nextPrerenderFrame(quint64 frame) {
    for (quint64 i = frame; i < d->timeline->frameCount(); i++) {
        if (!d->prerenders.contains(i)) {
            return i;
        }
    }
    return d->timeline->frameCount();
}

void Prerenderer::preprerenderNextFrame(quint64 frame) {
    if (d->restartPrerenderJob) {
        //Terminate the prerender job and start again
        completePrerenderJob();
    } else {
        quint64 next = nextPrerenderFrame(frame);
        if (next < d->timeline->frameCount()) {
            prerenderNextFrame(next);
        } else {
            completePrerenderJob();
        }
    }
}

void Prerenderer::prerenderNextFrame(quint64 frame) {
    this->cacheFrame(frame)->then([ = ] {
        preprerenderNextFrame(frame);
    });
}

void Prerenderer::completePrerenderJob() {
    d->prerenderJobRunning = false;
    if (d->restartPrerenderJob) tryPrerenderAll();
}
