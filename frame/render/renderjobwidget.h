#ifndef RENDERJOBWIDGET_H
#define RENDERJOBWIDGET_H

#include <QWidget>
#include "renderjob.h"

namespace Ui {
    class RenderJobWidget;
}

struct RenderJobWidgetPrivate;
class RenderJobWidget : public QWidget
{
        Q_OBJECT

    public:
        explicit RenderJobWidget(RenderJobPtr job, QWidget *parent = nullptr);
        ~RenderJobWidget();

    private slots:
        void on_playRenderedButton_clicked();

        void on_openVideoLocationButton_clicked();

        void on_cancelRenderButton_clicked();

    private:
        Ui::RenderJobWidget *ui;
        RenderJobWidgetPrivate* d;

        void updateState();
};

#endif // RENDERJOBWIDGET_H
