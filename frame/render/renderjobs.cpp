#include "renderjobs.h"
#include "ui_renderjobs.h"

#include "rendercontroller.h"
#include "renderjobwidget.h"
#include <the-libs_global.h>

RenderJobs::RenderJobs(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RenderJobs)
{
    ui->setupUi(this);

    ui->titleLabel->setBackButtonShown(true);
    ui->leftPane->setFixedWidth(SC_DPI(300));
    ui->stackedWidget->setCurrentAnimation(tStackedWidget::Lift);

    for (RenderJobPtr job : RenderController::instance()->jobs()) {
        ui->jobsWidget->addItem(job->jobDisplayName());

        RenderJobWidget* widget = new RenderJobWidget(job);
        ui->stackedWidget->addWidget(widget);
    }
}

RenderJobs::~RenderJobs()
{
    delete ui;
}

void RenderJobs::on_titleLabel_backButtonClicked()
{
    emit done();
}

void RenderJobs::on_jobsWidget_currentRowChanged(int currentRow)
{
    ui->stackedWidget->setCurrentIndex(currentRow);
}
