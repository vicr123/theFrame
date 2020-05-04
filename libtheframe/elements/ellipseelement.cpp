#include "ellipseelement.h"

EllipseElement::EllipseElement() : Element()
{
    this->setStartValue("geometry", QRect(0, 0, 100, 100));
    this->setStartValue("backgroundColor", QColor(0, 100, 200));
    this->setStartValue("borderColor", QColor(0, 0, 0));
    this->setStartValue("borderWidth", 1.0);
    this->setStartValue("opacity", 1.0);
}


void EllipseElement::render(QPainter* painter, quint64 frame) const
{
    painter->save();

    QRect geometry = this->propertyValueForFrame("geometry", frame).toRect();
    QColor backgroundColor = this->propertyValueForFrame("backgroundColor", frame).value<QColor>();
    QColor borderColor = this->propertyValueForFrame("borderColor", frame).value<QColor>();
    double borderWidth = this->propertyValueForFrame("borderWidth", frame).toDouble();
    double opacity = this->propertyValueForFrame("opacity", frame).toDouble() * painter->opacity();

    geometry.translate(this->parentElement()->renderOffset(frame));

    painter->setOpacity(opacity);
    painter->setBrush(backgroundColor);
    painter->setPen(QPen(borderColor, borderWidth));
    painter->drawEllipse(geometry);

    Element::render(painter, frame);

    painter->restore();
}

QPoint EllipseElement::renderOffset(quint64 frame) const
{
    QRect geometry = this->propertyValueForFrame("geometry", frame).toRect();
    return geometry.topLeft() + this->parentElement()->renderOffset(frame);
}

QMap<QString, Element::PropertyType> EllipseElement::animatableProperties() const
{
    return {
        {QStringLiteral("geometry"), Rect},
        {QStringLiteral("backgroundColor"), Color},
        {QStringLiteral("borderColor"), Color},
        {QStringLiteral("borderWidth"), Double},
        {QStringLiteral("opacity"), Percentage}
    };
}

QMap<QString, Element::PropertyType> EllipseElement::staticProperties() const
{
    return {};
}

QString EllipseElement::propertyDisplayName(QString property) const
{
    if (property == "geometry") {
        return tr("Geometry");
    } else if (property == "backgroundColor") {
        return tr("Background Color");
    } else if (property == "borderColor") {
        return tr("Border Color");
    } else if (property == "borderWidth") {
        return tr("Border Width");
    } else if (property == "opacity") {
        return tr("Opacity");
    }
    return "";
}

QColor EllipseElement::propertyColor(QString property) const
{
    if (property == "geometry") {
        return QColor(14, 40, 37, 127);
    } else if (property == "backgroundColor") {
        return QColor(38, 40, 21, 127);
    } else if (property == "borderColor") {
        return QColor(40, 18, 35, 127);
    } else if (property == "borderWidth") {
        return QColor(38, 40, 21, 127);
    } else if (property == "opacity") {
        return QColor(39, 40, 23, 127);
    }
    return QColor();
}

QString EllipseElement::typeDisplayName() const
{
    return tr("Ellipse");
}

QVariantMap EllipseElement::propertyMetadata(QString property) const
{    if (property == "borderWidth") {
        return {
            {"minValue", 0.0},
            {"maxValue", 999.0}
        };
    };
    return QVariantMap();
}
