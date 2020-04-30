#include "renderpopover.h"
#include "ui_renderpopover.h"

#include "renderjob.h"
#include "rendercontroller.h"
#include <QFileDialog>
#include <tsettings.h>

struct RenderPopoverPrivate {
    RenderJobPtr renderJob;
    tSettings settings;

    bool renderJobStarted = false;
};

RenderPopover::RenderPopover(QByteArray projectFile, QString projectPath, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RenderPopover)
{
    ui->setupUi(this);
    d = new RenderPopoverPrivate();

    d->renderJob = RenderJobPtr(new RenderJob(projectFile, projectPath));
    connect(d->renderJob.data(), &RenderJob::outputFileNameChanged, this, [=](QString filename) {
        ui->renderFile->setText(filename);
    });
    connect(d->renderJob.data(), &RenderJob::ffmpegPathChanged, this, [=](QString ffmpeg) {
        ui->ffmpegCommand->setText(ffmpeg);
    });
    connect(d->renderJob.data(), &RenderJob::rendererChanged, this, [=](QString renderer) {
        ui->rendererPath->setText(renderer);
    });
    ui->rendererPath->setText(d->renderJob->renderer());

    ui->titleLabel->setBackButtonShown(true);
}

RenderPopover::~RenderPopover()
{
    delete d;
    delete ui;
}

void RenderPopover::on_browseForFileButton_clicked()
{
    QFileDialog* fileDialog = new QFileDialog(this->window());
    fileDialog->setAcceptMode(QFileDialog::AcceptSave);
    fileDialog->setNameFilters({tr("MP4 Videos (*.mp4)")});
    fileDialog->setWindowFlag(Qt::Sheet);
    fileDialog->setWindowModality(Qt::WindowModal);
    connect(fileDialog, &QFileDialog::finished, this, [ = ](int result) {
        if (result == QFileDialog::Accepted) {
            d->renderJob->setOutputFileName(fileDialog->selectedFiles().first());
        }
        fileDialog->deleteLater();
    });
    fileDialog->open();
}

void RenderPopover::on_titleLabel_backButtonClicked()
{
    emit done();
}

void RenderPopover::on_browserForFfmpegButton_clicked()
{
    QFileDialog* fileDialog = new QFileDialog(this->window());
    fileDialog->setAcceptMode(QFileDialog::AcceptOpen);
#ifdef Q_OS_WIN
    fileDialog->setNameFilters({tr("FFMPEG (ffmpeg.exe)")});
#else
    fileDialog->setNameFilters({tr("FFMPEG (ffmpeg)")});
#endif
    fileDialog->setWindowFlag(Qt::Sheet);
    fileDialog->setWindowModality(Qt::WindowModal);
    connect(fileDialog, &QFileDialog::finished, this, [ = ](int result) {
        if (result == QFileDialog::Accepted) {
            d->renderJob->setFfmpegPath(fileDialog->selectedFiles().first());
        }
        fileDialog->deleteLater();
    });
    fileDialog->open();
}

void RenderPopover::on_browseForRenderer_clicked()
{
    QFileDialog* fileDialog = new QFileDialog(this->window());
    fileDialog->setAcceptMode(QFileDialog::AcceptOpen);
#ifdef Q_OS_WIN
    fileDialog->setNameFilters({tr("theFrame Renderer (theframe-render.exe)")});
#else
    fileDialog->setNameFilters({tr("theFrame Rnederer (theframe-render)")});
#endif
    fileDialog->setWindowFlag(Qt::Sheet);
    fileDialog->setWindowModality(Qt::WindowModal);
    connect(fileDialog, &QFileDialog::finished, this, [ = ](int result) {
        if (result == QFileDialog::Accepted) {
            d->renderJob->setRenderer(fileDialog->selectedFiles().first());
        }
        fileDialog->deleteLater();
    });
    fileDialog->open();
}

void RenderPopover::on_renderFile_textChanged(const QString &arg1)
{
    d->renderJob->setOutputFileName(arg1);
}

void RenderPopover::on_ffmpegCommand_textChanged(const QString &arg1)
{
    d->renderJob->setFfmpegPath(arg1);
}

void RenderPopover::on_rendererPath_textEdited(const QString &arg1)
{
    d->renderJob->setRenderer(arg1);
}

void RenderPopover::on_startRenderButton_clicked()
{
    RenderController::instance()->queueRenderJob(d->renderJob);
    emit renderingStarted(d->renderJob);
    emit done();
}
