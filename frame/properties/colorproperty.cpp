#include "colorproperty.h"
#include "ui_colorproperty.h"

#include <QColorDialog>

ColorProperty::ColorProperty(QWidget *parent) :
    PropertyWidget(parent),
    ui(new Ui::ColorProperty)
{
    ui->setupUi(this);
}

ColorProperty::~ColorProperty()
{
    delete ui;
}

void ColorProperty::setValue(QVariant value)
{
    PropertyWidget::setValue(value);

    QPalette pal = ui->colorButton->palette();
    pal.setColor(QPalette::Button, value.value<QColor>());
    ui->colorButton->setPalette(pal);
}

void ColorProperty::on_colorButton_clicked()
{
    QColorDialog* dialog = new QColorDialog(this->window());
    dialog->setCurrentColor(this->value().value<QColor>());
    dialog->setOption(QColorDialog::ShowAlphaChannel);
    dialog->setWindowTitle(tr("Select Color"));
    dialog->setWindowFlag(Qt::Sheet);
    dialog->setWindowModality(Qt::WindowModal);
    connect(dialog, &QColorDialog::finished, this, [=](int result) {
        if (result == QColorDialog::Accepted) {
            this->setValue(dialog->currentColor());
        }
        dialog->deleteLater();
    });
    dialog->open();
}
