#include "renderjobwidget.h"
#include "ui_renderjobwidget.h"

#include <QDesktopServices>
#include <QUrl>
#include <QProcess>
#include <QDir>

struct RenderJobWidgetPrivate {
    RenderJobPtr job;
};

RenderJobWidget::RenderJobWidget(RenderJobPtr job, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RenderJobWidget)
{
    ui->setupUi(this);

    d = new RenderJobWidgetPrivate();
    d->job = job;

    ui->titleLabel->setText(d->job->jobDisplayName());

    connect(d->job.data(), &RenderJob::progressChanged, this, [=](quint64 progress, quint64 maxProgress) {
        ui->progressBar->setMaximum(maxProgress);
        ui->progressBar->setValue(progress);
    });
    ui->progressBar->setMaximum(d->job->maxProgress());
    ui->progressBar->setValue(d->job->progress());

    ui->cancelRenderButton->setProperty("type", "destructive");

    connect(d->job.data(), &RenderJob::stateChanged, this, &RenderJobWidget::updateState);
    this->updateState();
}

RenderJobWidget::~RenderJobWidget()
{
    delete d;
    delete ui;
}

void RenderJobWidget::updateState()
{
    switch (d->job->state()) {
        case RenderJob::Idle:
            ui->stateLabel->setText(tr("Pending..."));
            ui->progressBar->setVisible(false);
            ui->jobCompleteWidget->setVisible(false);
            ui->jobRunningWidget->setVisible(false);
        case RenderJob::Started:
            ui->stateLabel->setText(tr("Currently rendering..."));
            ui->progressBar->setVisible(true);
            ui->jobCompleteWidget->setVisible(false);
            ui->jobRunningWidget->setVisible(true);
            break;
        case RenderJob::Finished:
            ui->stateLabel->setText(tr("Job Complete"));
            ui->progressBar->setVisible(false);
            ui->jobCompleteWidget->setVisible(true);
            ui->jobRunningWidget->setVisible(false);
            break;
        case RenderJob::Errored:
            ui->stateLabel->setText(tr("Job Failed"));
            ui->progressBar->setVisible(false);
            ui->jobCompleteWidget->setVisible(false);
            ui->jobRunningWidget->setVisible(false);
            break;
        case RenderJob::Cancelled:
            ui->stateLabel->setText(tr("Job Cancelled"));
            ui->progressBar->setVisible(false);
            ui->jobCompleteWidget->setVisible(false);
            ui->jobRunningWidget->setVisible(false);
            break;
    }
}

void RenderJobWidget::on_playRenderedButton_clicked()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(d->job->outputFileName()));
}

void RenderJobWidget::on_openVideoLocationButton_clicked()
{
#if defined(Q_OS_WIN)
    QProcess::startDetached("explorer.exe", {"/select,", QDir::toNativeSeparators(d->job->outputFileName())});
#elif defined(Q_OS_MAC)
    QProcess::execute("/usr/bin/osascript", {"-e", "tell application \"Finder\" to reveal POSIX file \"" + d->job->outputFileName() + "\""});
    QProcess::execute("/usr/bin/osascript", {"-e", "tell application \"Finder\" to activate"});
#else
    //not sure what to do on Linux :(
#endif
}

void RenderJobWidget::on_cancelRenderButton_clicked()
{
    d->job->cancelRenderJob();
}
