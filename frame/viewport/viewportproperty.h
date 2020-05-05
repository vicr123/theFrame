#ifndef VIEWPORTPROPERTY_H
#define VIEWPORTPROPERTY_H

#include <QWidget>
#include <QVariant>
#include <elements/element.h>

struct ViewportPropertyPrivate;
class Viewport;
class ViewportProperty : public QWidget
{
        Q_OBJECT
    public:
        enum Type {
            StartType,
            EndType,
            StartValueType
        };

        explicit ViewportProperty(Type type, Viewport *parent);
        ~ViewportProperty();

        virtual void setValue(QVariant value) = 0;
        virtual QVariant value() = 0;

        virtual void setAnchored(bool anchored) = 0;
        virtual void setOffset(QPoint offset) = 0;

        static ViewportProperty* constructForType(Element::PropertyType propertyType, Type type, Viewport* viewport);

    signals:
        void valueChanged(QVariant value);
        void focusFrame();

    private:
        ViewportPropertyPrivate* d;

};

#endif // VIEWPORTPROPERTY_H
