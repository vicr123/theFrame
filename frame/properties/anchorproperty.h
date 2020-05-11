#ifndef ANCHORPROPERTY_H
#define ANCHORPROPERTY_H

#include "propertywidget.h"

namespace Ui {
    class AnchorProperty;
}

struct AnchorPropertyPrivate;
class AnchorProperty : public PropertyWidget
{
        Q_OBJECT

    public:
        explicit Q_INVOKABLE AnchorProperty(QWidget *parent = nullptr);
        ~AnchorProperty();

        void setValue(QVariant value);

    private slots:
        void on_topLeft_toggled(bool checked);

        void on_top_toggled(bool checked);

        void on_topRight_toggled(bool checked);

        void on_left_toggled(bool checked);

        void on_center_toggled(bool checked);

        void on_right_toggled(bool checked);

        void on_bottomLeft_toggled(bool checked);

        void on_bottom_toggled(bool checked);

        void on_bottomRight_toggled(bool checked);

    private:
        Ui::AnchorProperty *ui;
        AnchorPropertyPrivate* d;
};

#endif // ANCHORPROPERTY_H
