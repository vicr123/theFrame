#include "renderpopover.h"
#include "ui_renderpopover.h"

#include "renderjob.h"
#include "rendercontroller.h"
#include <QFileDialog>
#include <QProcess>
#include <tsettings.h>
#include "ffmpegdetector.h"

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
        ensureSettingsValid();
    });

    connect(d->renderJob.data(), &RenderJob::videoCodecChanged, this, &RenderPopover::updateCodecCapabilities);
    updateCodecCapabilities();

    connect(&d->settings, &tSettings::settingChanged, this, &RenderPopover::settingChanged);

    QString renderer = d->settings.value("Render/rendererPath").toString();
    if (renderer == "theframe-render") {
#if defined(Q_OS_WIN)
        renderer = QApplication::applicationDirPath() + "/theframe-render.exe";
#elif defined(Q_OS_MAC)
        renderer = QApplication::applicationDirPath() + "/theframe-render";
#endif
    }
    d->settings.setValue("Render/rendererPath", renderer);

    this->settingChanged("Render/rendererPath", renderer);
    this->settingChanged("Render/ffmpegLocation", d->settings.value("Render/ffmpegLocation"));
    this->settingChanged("Render/ffmpegAutomatic", d->settings.value("Render/ffmpegAutomatic"));

    connect(FFmpegDetector::instance(), &FFmpegDetector::ffmpegDownloadingChanged, this, &RenderPopover::ffmpegDownloadStateChanged);
    connect(FFmpegDetector::instance(), &FFmpegDetector::ffmpegDownloadProgressChanged, this, &RenderPopover::ffmpegDownloadStateChanged);
    ffmpegDownloadStateChanged();

    ui->titleLabel->setBackButtonShown(true);
    ui->leftWidget->setFixedWidth(SC_DPI(300));
    ui->stackedWidget->setCurrentAnimation(tStackedWidget::Lift);
    ui->removeFfmpegButton->setProperty("type", "destructive");
}

RenderPopover::~RenderPopover()
{
    delete d;
    delete ui;
}

void RenderPopover::shown()
{
#ifdef Q_OS_MAC
    setupMacOS();
#endif
}

void RenderPopover::on_browseForFileButton_clicked()
{
    QFileDialog* fileDialog = new QFileDialog(this->window());
    fileDialog->setAcceptMode(QFileDialog::AcceptSave);
    fileDialog->setNameFilters(d->renderJob->videoCodecNameFilters());
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
            d->settings.setValue("Render/ffmpegLocation", fileDialog->selectedFiles().first());
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
    fileDialog->setNameFilters({tr("theFrame Renderer (theframe-render)")});
#endif
    fileDialog->setWindowFlag(Qt::Sheet);
    fileDialog->setWindowModality(Qt::WindowModal);
    connect(fileDialog, &QFileDialog::finished, this, [ = ](int result) {
        if (result == QFileDialog::Accepted) {
            d->settings.setValue("Render/rendererPath", fileDialog->selectedFiles().first());
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
    d->settings.setValue("Render/ffmpegLocation", arg1);
}

void RenderPopover::on_rendererPath_textEdited(const QString &arg1)
{
    d->settings.setValue("Render/rendererPath", arg1);
}

void RenderPopover::on_startRenderButton_clicked()
{
    ui->actionStart_Rendering->trigger();
}

void RenderPopover::on_ffmpegTabWidget_currentChanged(int index)
{
    if (index == 0) {
        d->settings.setValue("Render/ffmpegAutomatic", true);
    } else {
        d->settings.setValue("Render/ffmpegAutomatic", false);
    }

    ensureSettingsValid();
}

void RenderPopover::settingChanged(QString key, QVariant value)
{
    if (key == "Render/ffmpegLocation") {
        ui->ffmpegCommand->setText(value.toString());
    } else if (key == "Render/rendererPath") {
        ui->rendererPath->setText(value.toString());
    } else if (key == "Render/ffmpegAutomatic") {
        ui->ffmpegTabWidget->setCurrentIndex(value.toBool() ? 0 : 1);
    }

    ensureSettingsValid();
}

void RenderPopover::ffmpegDownloadStateChanged()
{
    if (FFmpegDetector::instance()->isFfmpegDownloading()) {
        ui->ffmpegDownloadStackedWidget->setCurrentIndex(1);
        ui->ffmpegProgressBar->setMaximum(FFmpegDetector::instance()->totalProgress());
        ui->ffmpegProgressBar->setValue(FFmpegDetector::instance()->currentProgress());
    } else {
        ui->ffmpegDownloadStackedWidget->setCurrentIndex(0);
        ensureSettingsValid();
    }
}

void RenderPopover::ensureSettingsValid()
{
    bool ffmpegOk = false;
    auto checkFfmpeg = [=](QString ffmpegPath) -> QString {
        if (ffmpegPath.isEmpty()) return "";

        QProcess proc;
        proc.start(ffmpegPath, {"-version"});
        proc.waitForStarted();
        proc.waitForFinished();

        if (proc.exitCode() == 0) {
            QString output = proc.readAll();
            QStringList firstLine = output.split("\n").first().split(" ");
            if (firstLine.count() >= 3) {
                return tr("Current FFmpeg version: %1").arg(firstLine.at(2));
            } else {
                return tr("FFmpeg is working.");
            }
        } else {
            return "";
        }
    };

    if (d->settings.value("Render/ffmpegAutomatic").toBool()) {
        if (FFmpegDetector::instance()->isFfmpegDownloadError()) {
            ui->ffmpegVersionLabel->setText(tr("FFmpeg failed to download."));
            if (FFmpegDetector::instance()->renderFfmpegAvailable()) {
                ffmpegOk = true;
            }
        } else {
            if (FFmpegDetector::instance()->renderFfmpegAvailable()) {
                QString ffmpegVersion = checkFfmpeg(FFmpegDetector::instance()->renderFfmpegPath());
                if (ffmpegVersion.isEmpty()) {
                    //Render FFMPEG is not working correctly
                    //TODO: Offer to redownload if this is the theFrame FFMpeg
                    ui->ffmpegVersionLabel->setText(tr("Automatic FFmpeg is not working correctly. Manual FFmpeg settings are required to render correctly."));
                } else {
                    ui->ffmpegVersionLabel->setText(ffmpegVersion);
                    ffmpegOk = true;
                }
            } else {
#ifdef T_OS_UNIX_NOT_MAC
                ui->ffmpegVersionLabel->setText(tr("Install FFmpeg using your system package manager, and then restart theFrame to continue rendering."));
#else
                ui->ffmpegVersionLabel->setText(tr("FFmpeg has not been downloaded."));
#endif
            }
        }

        ui->removeFfmpegButton->setVisible(FFmpegDetector::instance()->theframeFfmpegAvailable());
    } else {
        QString ffmpegVersion = checkFfmpeg(d->settings.value("Render/ffmpegLocation").toString());
        if (!ffmpegVersion.isEmpty()) {
            ui->manualFfmpegVersion->setText(ffmpegVersion);
            ffmpegOk = true;
        } else {
            ui->manualFfmpegVersion->setText("");
        }
    }

    bool fileOk = true;
    if (d->renderJob->outputFileName().isEmpty()) fileOk = false;
    if (!QFileInfo(d->renderJob->outputFileName()).dir().exists()) fileOk = false;

    auto checkRenderer = [=](QString rendererPath) {
        if (rendererPath.isEmpty()) return false;

        QProcess proc;
        proc.start(rendererPath, {"-v"});
        proc.waitForFinished();

        return proc.exitCode() == 0;
    };
    bool rendererOk = false;
    if (checkRenderer(d->settings.value("Render/rendererPath").toString())) rendererOk = true;

    bool canRender = ffmpegOk && fileOk && rendererOk;
    ui->actionStart_Rendering->setEnabled(canRender);
    ui->renderUnavailablePrompt->setVisible(!canRender);

    if (!canRender) {
        QStringList explanation;
        explanation.append(tr("Looks like there's an issue with some settings. You'll need to fix them before we can start rendering.") + "\n");

        if (!fileOk) explanation.append("- " + tr("Output file needs to be reviewed"));
        if (!ffmpegOk) explanation.append("- " + tr("FFmpeg settings need to be reviewed"));
        if (!rendererOk) explanation.append("- " + tr("Renderer is unavailable"));

        ui->renderUnavailableExplanation->setText(explanation.join("\n"));
    }
}

void RenderPopover::updateCodecCapabilities()
{
    QList<RenderJob::CodecCapability> capabilities = d->renderJob->videoCodecCapabilities();
    ui->translucentBackgroundCheckbox->setVisible(capabilities.contains(RenderJob::Transparency));
}

void RenderPopover::on_downloadFfmpegButton_clicked()
{
    FFmpegDetector::instance()->downloadFfmpeg();
}

void RenderPopover::on_removeFfmpegButton_clicked()
{
    FFmpegDetector::instance()->removeFfmpeg();
    ensureSettingsValid();
}

void RenderPopover::on_actionStart_Rendering_triggered()
{
    RenderController::instance()->queueRenderJob(d->renderJob);
    emit renderingStarted(d->renderJob);
    emit done();
}

void RenderPopover::on_actionStart_Rendering_changed()
{
    ui->startRenderButton->setEnabled(ui->actionStart_Rendering->isEnabled());
}

void RenderPopover::on_leftList_currentRowChanged(int currentRow)
{
    ui->stackedWidget->setCurrentIndex(currentRow);
}

void RenderPopover::on_formatBox_currentIndexChanged(int index)
{
    d->renderJob->setVideoCodec(static_cast<RenderJob::VideoCodec>(index));
}

void RenderPopover::on_translucentBackgroundCheckbox_toggled(bool checked)
{
    if (checked) {
        d->renderJob->setCodecCapabilityEnabled(RenderJob::Transparency, checked);
    }
}
