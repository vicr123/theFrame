#include "rectangleviewport.h"

#include "viewport.h"
#include <QPainter>
#include <QMouseEvent>

struct RectangleViewportPrivate {
    Viewport* parent;
    RectangleViewport::Type type;
    QRect value;
    QColor color;
    QPoint offset;
    bool anchored;

    bool isMouseDown = false;
    RectangleViewport::SizingFlags sizingFlags;
    QRect initialState;
    QPoint initialPos;

    QRect targetGeometry;

    const int lineWidth = SC_DPI(5);
};

RectangleViewport::RectangleViewport(Type type, Viewport *parent) : QWidget(parent)
{
    d = new RectangleViewportPrivate();
    d->parent = parent;
    d->type = type;

    switch (type) {
        case RectangleViewport::StartType:
            d->color = Qt::red;
            break;
        case RectangleViewport::EndType:
            d->color = Qt::blue;
            break;

    }

    connect(parent, &Viewport::viewportRectChanged, this, &RectangleViewport::updateGeometry);
    this->setMouseTracking(true);

}

RectangleViewport::~RectangleViewport()
{
    this->setVisible(false);
    this->setParent(nullptr);
    delete d;
}

void RectangleViewport::setValue(QRect value)
{
    d->value = value;
    emit valueChanged(value);

    this->updateGeometry();
}

QRect RectangleViewport::value()
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
    this->setVisible(d->anchored);
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
    painter.setBrush(d->color);
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
                newGeometry.translate(event->globalPos() - d->initialPos);
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
                break;
            case Right | Bottom:
                newGeometry.setBottomRight(parentPos);
                break;
            case Left | Bottom:
                newGeometry.setBottomLeft(parentPos);
                break;
            case Right | Top:
                newGeometry.setTopRight(parentPos);
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
