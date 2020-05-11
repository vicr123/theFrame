#include "rectangleviewport.h"

#include "viewport.h"
#include <QPainter>
#include <QMouseEvent>
#include <tvariantanimation.h>

struct RectangleViewportPrivate {
    Viewport* parent;
    ViewportProperty::Type type;
    QRect value;
    tVariantAnimation* color;
    QPoint offset;
    bool anchored = false;

    bool isMouseDown = false;
    RectangleViewport::SizingFlags sizingFlags;
    QRect initialState;
    QPoint initialPos;

    QRect targetGeometry;

    const int lineWidth = SC_DPI(5);
};

RectangleViewport::RectangleViewport(Type type, Viewport *parent) : ViewportProperty(type, parent)
{
    d = new RectangleViewportPrivate();
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

    connect(parent, &Viewport::viewportRectChanged, this, &RectangleViewport::updateGeometry);
    this->setMouseTracking(true);

}

RectangleViewport::~RectangleViewport()
{
    this->setVisible(false);
    this->setParent(nullptr);
    delete d;
}

void RectangleViewport::setValue(QVariant value)
{
    d->value = value.toRect();
    emit valueChanged(value);

    this->updateGeometry();
}

QVariant RectangleViewport::value()
{
    return d->value;
}

void RectangleViewport::setAnchored(bool anchored)
{
    d->anchored = anchored;
    this->setVisible(!anchored);
}

void RectangleViewport::setOffset(QPoint offset)
{
    d->offset = offset;
}

void RectangleViewport::updateGeometry()
{
    QRect newGeometry = d->parent->canvasToViewport(d->value.translated(d->offset));
    d->targetGeometry = newGeometry;

    newGeometry.adjust(-d->lineWidth, -d->lineWidth, d->lineWidth, d->lineWidth);

    this->setGeometry(newGeometry);
    this->setVisible(!d->anchored);
}

RectangleViewport::SizingFlags RectangleViewport::hitTest(QPoint point)
{
    SizingFlags flags = Body;
    if (point.x() < d->lineWidth) flags |= Left;
    if (point.y() < d->lineWidth) flags |= Top;
    if (point.x() > this->width() - d->lineWidth) flags |= Right;
    if (point.y() > this->height() - d->lineWidth) flags |= Bottom;
    return flags;
}

void RectangleViewport::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setPen(Qt::transparent);
    painter.setBrush(d->color->currentValue().value<QColor>());
    painter.drawRect(0, 0, this->width(), d->lineWidth);
    painter.drawRect(0, 0, d->lineWidth, this->height());
    painter.drawRect(0, this->height() - d->lineWidth, this->width(), d->lineWidth);
    painter.drawRect(this->width() - d->lineWidth, 0, d->lineWidth, this->height());
}

void RectangleViewport::mousePressEvent(QMouseEvent* event)
{
    d->isMouseDown = true;
    d->sizingFlags = this->hitTest(event->pos());
    d->initialState = d->targetGeometry;
    d->initialPos = event->globalPos();
}

void RectangleViewport::mouseMoveEvent(QMouseEvent* event)
{
    if (d->isMouseDown) {
        QRect newGeometry = d->initialState;
        QPoint parentPos = this->mapTo(d->parent, event->pos());
        switch (d->sizingFlags) {
            case Body:
                if (event->modifiers() & Qt::ShiftModifier) {
                    //Constrain to an axis
                    QPoint difference = event->globalPos() - d->initialPos;
                    if (qAbs(difference.y()) > qAbs(difference.x())) {
                        //Constrain to the Y axis
                        newGeometry.translate(0, difference.y());
                    } else {
                        //Constrain to the X axis
                        newGeometry.translate(difference.x(), 0);
                    }
                } else {
                    //Translate normally
                    newGeometry.translate(event->globalPos() - d->initialPos);
                }
                break;
            case Left:
                newGeometry.setLeft(parentPos.x());
                break;
            case Right:
                newGeometry.setRight(parentPos.x());
                break;
            case Top:
                newGeometry.setTop(parentPos.y());
                break;
            case Bottom:
                newGeometry.setBottom(parentPos.y());
                break;
            case Left | Top:
                newGeometry.setTopLeft(parentPos);
                if (event->modifiers() & Qt::ShiftModifier) {
                    //Constrain ratio
                    QPoint fixedPoint = newGeometry.bottomRight();
                    newGeometry.setSize(d->initialState.size().scaled(newGeometry.size(), Qt::KeepAspectRatio));
                    newGeometry.moveBottomRight(fixedPoint);
                }
                break;
            case Right | Bottom:
                newGeometry.setBottomRight(parentPos);
                if (event->modifiers() & Qt::ShiftModifier) {
                    //Constrain ratio
                    QPoint fixedPoint = newGeometry.topLeft();
                    newGeometry.setSize(d->initialState.size().scaled(newGeometry.size(), Qt::KeepAspectRatio));
                    newGeometry.moveTopLeft(fixedPoint);
                }
                break;
            case Left | Bottom:
                newGeometry.setBottomLeft(parentPos);
                if (event->modifiers() & Qt::ShiftModifier) {
                    //Constrain ratio
                    QPoint fixedPoint = newGeometry.topRight();
                    newGeometry.setSize(d->initialState.size().scaled(newGeometry.size(), Qt::KeepAspectRatio));
                    newGeometry.moveTopRight(fixedPoint);
                }
                break;
            case Right | Top:
                newGeometry.setTopRight(parentPos);
                if (event->modifiers() & Qt::ShiftModifier) {
                    //Constrain ratio
                    QPoint fixedPoint = newGeometry.bottomLeft();
                    newGeometry.setSize(d->initialState.size().scaled(newGeometry.size(), Qt::KeepAspectRatio));
                    newGeometry.moveBottomLeft(fixedPoint);
                }
                break;
        }

        this->setValue(d->parent->viewportToCanvas(newGeometry.normalized()).translated(-d->offset));
    } else {
        SizingFlags flags = this->hitTest(event->pos());
        switch (flags) {
            case Body:
                this->setCursor(QCursor(Qt::ArrowCursor));
                break;
            case Left:
            case Right:
                this->setCursor(QCursor(Qt::SizeHorCursor));
                break;
            case Top:
            case Bottom:
                this->setCursor(QCursor(Qt::SizeVerCursor));
                break;
            case Left | Top:
            case Right | Bottom:
                this->setCursor(QCursor(Qt::SizeFDiagCursor));
                break;
            case Left | Bottom:
            case Right | Top:
                this->setCursor(QCursor(Qt::SizeBDiagCursor));
                break;
        }
    }
}

void RectangleViewport::mouseReleaseEvent(QMouseEvent* event)
{
    d->isMouseDown = false;
}

void RectangleViewport::mouseDoubleClickEvent(QMouseEvent* event)
{
    emit focusFrame();
}
