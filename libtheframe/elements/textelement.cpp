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
#include "textelement.h"

#include <QApplication>

TextElement::TextElement() : Element() {
    this->setStartValue("text", tr("Edit Me!"));
    this->setStartValue("textColor", QColor(0, 0, 0));
    this->setStartValue("position", QPoint(100, 100));
    this->setStartValue("font", QApplication::font());
    this->setStartValue("fontSize", 50.0);
    this->setStartValue("opacity", 1.0);
}


void TextElement::render(QPainter* painter, quint64 frame) const {
    painter->save();

    QString text = this->startValue("text").toString();
    QFont font = this->startValue("font").value<QFont>();
    double fontSize = this->propertyValueForFrame("fontSize", frame).toDouble();
    QColor textCol = this->propertyValueForFrame("textColor", frame).value<QColor>();
    QPoint pos = this->propertyValueForFrame("position", frame).toPoint();
    double opacity = this->propertyValueForFrame("opacity", frame).toDouble();

    font.setPointSizeF(fontSize);
    QFontMetrics metrics(font);

    pos += this->parentElement()->renderOffset(frame);

    QRect textRect;
    textRect.setHeight(metrics.height());
    textRect.setWidth(metrics.horizontalAdvance(text));
    textRect.moveCenter(pos); //TODO: Anchor


    painter->setOpacity(painter->opacity() * opacity);
    painter->setFont(font);
    painter->setPen(textCol);
    painter->drawText(textRect, Qt::AlignCenter, text);

    Element::render(painter, frame);

    painter->restore();
}

QPoint TextElement::renderOffset(quint64 frame) const {
    QPoint position = this->propertyValueForFrame("position", frame).toPoint();
    return position + this->parentElement()->renderOffset(frame);
}

QMap<QString, Element::PropertyType> TextElement::animatableProperties() const {
    return {
        {"fontSize", Element::Double},
        {"position", Element::Point},
        {"textColor", Element::Color},
        {"opacity", Element::Percentage}
    };
}

QMap<QString, Element::PropertyType> TextElement::staticProperties() const {
    return {
        {"text", Element::String},
        {"font", Element::Font}
    };
}

QString TextElement::propertyDisplayName(QString property) const {
    if (property == "text") {
        return tr("Text");
    } else if (property == "position") {
        return tr("Position");
    } else if (property == "fontSize") {
        return tr("Font Size");
    } else if (property == "textColor") {
        return tr("Text Color");
    } else if (property == "font") {
        return tr("Font");
    } else if (property == "opacity") {
        return tr("Opacity");
    }
    return QString();
}

QColor TextElement::propertyColor(QString property) const {
    if (property == "text") {
        return QColor(14, 40, 37, 127);
    } else if (property == "position") {
        return QColor(38, 40, 21, 127);
    } else if (property == "fontSize") {
        return QColor(14, 40, 37, 127);
    } else if (property == "textColor") {
        return QColor(40, 18, 35, 127);
    } else if (property == "font") {
        return QColor(39, 40, 23, 127);
    } else if (property == "opacity") {
        return QColor(14, 40, 37, 127);
    }
    return QColor();
}

QString TextElement::typeDisplayName() const {
    return tr("Text");
}


QVariantMap TextElement::propertyMetadata(QString property) const
{
    if (property == "fontSize") {
        return {
            {"minValue", 1.0},
            {"maxValue", 999.0}
        };
    }
    return QVariantMap();
}
