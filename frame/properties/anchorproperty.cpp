#include "anchorproperty.h"
#include "ui_anchorproperty.h"

#include <elements/element.h>

struct AnchorPropertyPrivate {
    QMap<Element::AnchorPosition, QPushButton*> buttons;
};

AnchorProperty::AnchorProperty(QWidget *parent) :
    PropertyWidget(parent),
    ui(new Ui::AnchorProperty)
{
    ui->setupUi(this);
    d = new AnchorPropertyPrivate();
    d->buttons = {
        {Element::TopLeft, ui->topLeft},
        {Element::TopCenter, ui->top},
        {Element::TopRight, ui->topRight},
        {Element::CenterLeft, ui->left},
        {Element::Center, ui->center},
        {Element::CenterRight, ui->right},
        {Element::BottomLeft, ui->bottomLeft},
        {Element::BottomCenter, ui->bottom},
        {Element::BottomRight, ui->bottomRight}
    };
}

AnchorProperty::~AnchorProperty()
{
    delete d;
    delete ui;
}

void AnchorProperty::setValue(QVariant value)
{
    PropertyWidget::setValue(value);

    for (QPushButton* button : d->buttons.values()) {
        button->setChecked(false);
    }
    d->buttons.value(value.value<Element::AnchorPosition>())->setChecked(true);
}

void AnchorProperty::on_topLeft_toggled(bool checked)
{
    if (checked) {
        this->setValue(Element::TopLeft);
    }
}

void AnchorProperty::on_top_toggled(bool checked)
{
    if (checked) {
        this->setValue(Element::TopCenter);
    }
}

void AnchorProperty::on_topRight_toggled(bool checked)
{
    if (checked) {
        this->setValue(Element::TopRight);
    }
}

void AnchorProperty::on_left_toggled(bool checked)
{
    if (checked) {
        this->setValue(Element::CenterLeft);
    }
}

void AnchorProperty::on_center_toggled(bool checked)
{
    if (checked) {
        this->setValue(Element::Center);
    }
}

void AnchorProperty::on_right_toggled(bool checked)
{
    if (checked) {
        this->setValue(Element::CenterRight);
    }
}

void AnchorProperty::on_bottomLeft_toggled(bool checked)
{
    if (checked) {
        this->setValue(Element::BottomLeft);
    }
}

void AnchorProperty::on_bottom_toggled(bool checked)
{
    if (checked) {
        this->setValue(Element::BottomCenter);
    }
}

void AnchorProperty::on_bottomRight_toggled(bool checked)
{
    if (checked) {
        this->setValue(Element::BottomRight);
    }
}
