#ifndef POINTVIEWPORT_H
#define POINTVIEWPORT_H

#include "viewportproperty.h"

struct PointViewportPrivate;
class PointViewport : public ViewportProperty
{
        Q_OBJECT
    public:

        explicit PointViewport(Type type, Viewport *parent = nullptr);
        ~PointViewport();

        void setValue(QVariant value);
        QVariant value();

        void setAnchored(bool anchored);
        void setOffset(QPoint offset);

    signals:

    private:
        friend PointViewportPrivate;
        PointViewportPrivate* d;

        void updateGeometry();

        void paintEvent(QPaintEvent *event);
        void mousePressEvent(QMouseEvent *event);
        void mouseMoveEvent(QMouseEvent *event);
        void mouseReleaseEvent(QMouseEvent *event);
        void mouseDoubleClickEvent(QMouseEvent *event);
};

#endif // POINTVIEWPORT_H
