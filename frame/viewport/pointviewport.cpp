#include "pointviewport.h"

#include "viewport.h"
#include "tvariantanimation.h"

#include <QMouseEvent>

struct PointViewportPrivate {
    Viewport* parent;
    ViewportProperty::Type type;
    QPoint value;
    tVariantAnimation* color;
    QPoint offset;
    bool anchored = false;

    QPoint targetPoint;

    bool isMouseDown;
    QPoint initialState;
    QPoint initialPos;
};

PointViewport::PointViewport(ViewportProperty::Type type, Viewport* parent) : ViewportProperty(type, parent)
{
    d = new PointViewportPrivate();
    d->parent = parent;
    d->type = type;

    d->color = new tVariantAnimation(this);
    d->color->setDuration(1000);

    switch (type) {
        case StartType:
            d->color->setStartValue(QColor(100, 0, 0));
            d->color->setEndValue(QColor(255, 0, 0));
            break;
        case EndType:
            d->color->setStartValue(QColor(0, 100, 255));
            d->color->setEndValue(QColor(0, 255, 255));
            break;
        case StartValueType:
            d->color->setStartValue(QColor(0, 100, 0));
            d->color->setEndValue(QColor(0, 255, 0));
    }

    connect(d->color, &tVariantAnimation::valueChanged, this, [=] {
        this->update();
    });
    connect(d->color, &tVariantAnimation::finished, this, [=] {
        d->color->setDirection(d->color->direction() == tVariantAnimation::Forward ? tVariantAnimation::Backward : tVariantAnimation::Forward);
        d->color->start();
    });
    d->color->start();

    connect(parent, &Viewport::viewportRectChanged, this, &PointViewport::updateGeometry);
    this->setMouseTracking(true);

}

PointViewport::~PointViewport()
{
    this->setVisible(false);
    this->setParent(nullptr);
    delete d;
}

void PointViewport::setValue(QVariant value)
{
    d->value = value.toPoint();
    emit valueChanged(value);

    this->updateGeometry();
}

QVariant PointViewport::value()
{
    return d->value;
}

void PointViewport::setAnchored(bool anchored)
{
    d->anchored = anchored;
    this->setVisible(!anchored);
}

void PointViewport::setOffset(QPoint offset)
{
    d->offset = offset;
}

void PointViewport::updateGeometry()
{
    d->targetPoint = d->parent->canvasToViewport(d->value + d->offset);

    QRect newGeometry;
    newGeometry.setSize(SC_DPI_T(QSize(10, 10), QSize));
    newGeometry.moveCenter(d->targetPoint);

    this->setGeometry(newGeometry);
    this->setVisible(!d->anchored);
}

void PointViewport::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setPen(Qt::transparent);
    painter.setBrush(d->color->currentValue().value<QColor>());
    painter.drawEllipse(0, 0, this->width(), this->height());
}

void PointViewport::mousePressEvent(QMouseEvent* event)
{
    d->isMouseDown = true;
    d->initialState = d->targetPoint;
    d->initialPos = event->globalPos();
}

void PointViewport::mouseMoveEvent(QMouseEvent* event)
{
    if (d->isMouseDown) {
        QPoint newPoint = d->initialState;
        newPoint += event->globalPos() - d->initialPos;
        this->setValue(d->parent->viewportToCanvas(newPoint) - d->offset);
    }
}

void PointViewport::mouseReleaseEvent(QMouseEvent* event)
{
    d->isMouseDown = false;
}

void PointViewport::mouseDoubleClickEvent(QMouseEvent* event)
{
    emit focusFrame();
}
