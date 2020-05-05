#ifndef RECTANGLEVIEWPORT_H
#define RECTANGLEVIEWPORT_H

#include "viewportproperty.h"

class Viewport;
struct RectangleViewportPrivate;
class TimelineElement;
class RectangleViewport : public ViewportProperty
{
        Q_OBJECT
    public:

        explicit RectangleViewport(Type type, Viewport *parent = nullptr);
        ~RectangleViewport();

        void setValue(QVariant value);
        QVariant value();

        void setAnchored(bool anchored);
        void setOffset(QPoint offset);

    signals:

    private:
        friend RectangleViewportPrivate;
        RectangleViewportPrivate* d;

        enum SizingFlag {
            Body = 0,
            Left = 1,
            Top = 2,
            Right = 4,
            Bottom = 8
        };
        typedef QFlags<SizingFlag> SizingFlags;

        void updateGeometry();
        SizingFlags hitTest(QPoint point);

        void paintEvent(QPaintEvent *event);
        void mousePressEvent(QMouseEvent *event);
        void mouseMoveEvent(QMouseEvent *event);
        void mouseReleaseEvent(QMouseEvent *event);
        void mouseDoubleClickEvent(QMouseEvent *event);
};

#endif // RECTANGLEVIEWPORT_H
