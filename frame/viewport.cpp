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
#include "viewport.h"
#include "ui_viewport.h"

#include <QMap>
#include "prerenderer.h"

struct ViewportPrivate {
    ViewportElement* rootElement;
    Prerenderer* prerenderer;

    quint64 currentFrame;
    QPixmap rootRender;

    bool renderingFrame = false;
    bool renderRequired = false;
};

Viewport::Viewport(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::Viewport) {
    ui->setupUi(this);

    d = new ViewportPrivate();
    d->rootElement = new ViewportElement();
    d->rootElement->setName(tr("Canvas"));

    connect(d->rootElement, &ViewportElement::invalidateFromFrame, this, [=](quint64 frame) {
        if (d->currentFrame >= frame) this->setFrame(d->currentFrame);
    });
}

Viewport::~Viewport() {
    d->rootElement->deleteLater();
    delete ui;
    delete d;
}

void Viewport::setFrame(quint64 frame) {
    d->currentFrame = frame;

    if (!d->renderingFrame) {
        d->renderingFrame = true;
        d->prerenderer->frame(frame)->then([ = ](QPixmap pixmap) {
            d->rootRender = pixmap;

            //Force a repaint
            this->repaint();

            d->renderingFrame = false;
            if (d->renderRequired) {
                d->renderRequired = false;
                this->setFrame(d->currentFrame);
            }
        });
    } else {
        d->renderRequired = true;
    }
}

void Viewport::setPrerenderer(Prerenderer* prerenderer) {
    d->prerenderer = prerenderer;
    connect(prerenderer, &Prerenderer::framePrerenderStateChanged, this, [ = ](quint64 frame) {
        if (frame == d->currentFrame) setFrame(frame);
    });
}

ViewportElement* Viewport::rootElement() {
    return d->rootElement;
}

void Viewport::paintEvent(QPaintEvent* event) {
    QPainter painter(this);

    QSize viewportSize = d->rootElement->viewportSize();
    QSize drawSize = viewportSize.scaled(this->width(), this->height(), Qt::KeepAspectRatio);

    QRect drawRect;
    drawRect.setSize(drawSize);
    drawRect.moveCenter(QPoint(this->width() / 2, this->height() / 2));
    painter.drawPixmap(drawRect, d->rootRender);
}
