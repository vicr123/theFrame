#ifndef COLORPROPERTY_H
#define COLORPROPERTY_H

#include "propertywidget.h"

namespace Ui {
    class ColorProperty;
}

class ColorProperty : public PropertyWidget
{
        Q_OBJECT

    public:
        Q_INVOKABLE explicit ColorProperty(QWidget *parent = nullptr);
        ~ColorProperty();

        void setValue(QVariant value);

    private slots:
        void on_colorButton_clicked();

    private:
        Ui::ColorProperty *ui;
};

#endif // COLORPROPERTY_H
