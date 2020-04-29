#include "renderjob.h"

#include <QApplication>
#include <QProcess>
#include <QFileInfo>

struct RenderJobPrivate {
    QString projectFile;
    QString filename;
    QString ffmpegPath = "ffmpeg";
    QString renderer;

    QProcess* renderProcess;

    quint64 progress = 0;
    quint64 maxProgress = 0;

    RenderJob::State state = RenderJob::Idle;
};

RenderJob::RenderJob(QString projectFile, QObject *parent) : QObject(parent)
{
    d = new RenderJobPrivate();
    d->projectFile = projectFile;

#if defined(Q_OS_WIN)
    d->renderer = QApplication::applicationDirPath() + "/theframe-render.exe";
#elif defined(Q_OS_MAC)
    d->renderer =  QApplication::applicationDirPath() + "/theframe-render";
#else
    d->renderer = "theframe-render";
#endif
}

RenderJob::~RenderJob()
{
    delete d;
}

void RenderJob::setOutputFileName(QString filename)
{
    d->filename = filename;
    emit outputFileNameChanged(filename);
}

QString RenderJob::outputFileName()
{
    return d->filename;
}

QString RenderJob::jobDisplayName()
{
    return QFileInfo(d->filename).fileName();
}

void RenderJob::setFfmpegPath(QString ffmpeg)
{
    d->ffmpegPath = ffmpeg;
    emit ffmpegPathChanged(ffmpeg);
}

QString RenderJob::ffmpegPath()
{
    return d->ffmpegPath;
}

void RenderJob::setRenderer(QString renderer)
{
    d->renderer = renderer;
    emit rendererChanged(renderer);
}

QString RenderJob::renderer()
{
    return d->renderer;
}

void RenderJob::startRenderJob()
{
    if (d->state != Idle) return;
    d->renderProcess = new QProcess(this);
    d->renderProcess->setProgram(d->renderer);
    d->renderProcess->setArguments({
        "--ffmpeg-command", d->ffmpegPath,
        d->projectFile,
        d->filename
    });
    connect(d->renderProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [=](int exitCode, QProcess::ExitStatus state) {
        if (exitCode == 0) {
            d->state = Finished;
        } else {
            d->state = Errored;
        }
        emit stateChanged(d->state);
    });
    connect(d->renderProcess, &QProcess::readyReadStandardOutput, this, [=] {
        QString output = d->renderProcess->readAllStandardOutput();
        for (QString line : output.split("\n")) {
            if (line.contains("/")) {
                QStringList parts = line.split("/");
                d->progress = parts.at(0).toULongLong();
                d->maxProgress = parts.at(1).toULongLong();

                if (d->progress == d->maxProgress) {
                    d->progress = 0;
                    d->maxProgress = 0;
                }
                emit progressChanged(d->progress, d->maxProgress);
            }
        }
    });
    d->renderProcess->start();

    if (!d->renderProcess->waitForStarted()) {
        d->state = Errored;
    } else {
        d->state = Started;
    }
    emit stateChanged(d->state);
}

RenderJob::State RenderJob::state()
{
    return d->state;
}

quint64 RenderJob::progress()
{
    return d->progress;
}

quint64 RenderJob::maxProgress()
{
    return d->maxProgress;
}
