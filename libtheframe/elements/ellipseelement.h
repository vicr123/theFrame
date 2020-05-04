#ifndef ELLIPSEELEMENT_H
#define ELLIPSEELEMENT_H

#include "element.h"
#include "libtheframe_global.h"

class LIBTHEFRAME_EXPORT EllipseElement : public Element
{
        Q_OBJECT
    public:
        explicit Q_INVOKABLE EllipseElement();

    signals:


        // Element interface
    public:
        void render(QPainter* painter, quint64 frame) const;
        QPoint renderOffset(quint64 frame) const;
        QMap<QString, PropertyType> animatableProperties() const;
        QMap<QString, PropertyType> staticProperties() const;
        QString propertyDisplayName(QString property) const;
        QColor propertyColor(QString property) const;
        QString typeDisplayName() const;
        QVariantMap propertyMetadata(QString property) const;
};

#endif // ELLIPSEELEMENT_H
