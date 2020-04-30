/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
 *   Copyright (C) 2020 Victor Tran
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * *************************************/
#include "renderer.h"

#include <QCommandLineParser>
#include <QTextStream>
#include <QFile>
#include <QSize>
#include <QTemporaryDir>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <QtConcurrent>
#include <elements/viewportelement.h>

#include <QProcess>

struct RendererPrivate {
    ViewportElement* viewport;
    QJsonObject rootObject;

    QByteArray input;
    QString projectPath;
    QString outputFile;

    QString ffmpegCommand;
    QString vcodec;

    uint framerate;
    quint64 frameCount;

    QTemporaryDir temporaryDir;
    bool stop = false;
};

Renderer::Renderer(QObject* parent) : QObject(parent) {
    d = new RendererPrivate();
}

Renderer::~Renderer() {
    delete d;
}

bool Renderer::prepare() {
    QCommandLineParser parser;
    parser.addPositionalArgument("project", tr("Project file to render"), tr("[project]"));
    parser.addPositionalArgument("output", tr("Output File"), tr("[output]"));
    parser.addOption({"ffmpeg-command", tr("FFmpeg command to run"), tr("ffmpeg-command")});
    parser.addOption({"vcodec", tr("Video codec to use"), tr("vcodec")});
    parser.addOption({"project-path", tr("Root path for project files"), tr("project-path")});
    parser.addHelpOption();
    parser.setOptionsAfterPositionalArgumentsMode(QCommandLineParser::ParseAsOptions);
    parser.process(QCoreApplication::instance()->arguments());

    if (parser.positionalArguments().count() < 2) {
        logError(tr("Missing arguments. Check out --help for help."));
        return false;
    }

    QString inputFile = parser.positionalArguments().first();
    if (parser.isSet("project-path")) {
        d->projectPath = parser.value("project-path");
    } else if (inputFile == "-") {
        logError(tr("project-path argument required if reading from stdin."));
        return false;
    } else {
        d->projectPath = QFileInfo(inputFile).path();
    }

    QByteArray inputContents;
    if (inputFile == "-") {
        //Read from stdin
        QFile f;
        f.open(stdin, QFile::ReadOnly);
        while (!f.atEnd()) {
            inputContents.append(f.readAll());
        }

    } else {
        if (!QFile::exists(inputFile)) {
            logError(tr("Source file not found."));
            return false;
        }

        QFile sourceFile(parser.positionalArguments().first());
        if (!sourceFile.open(QFile::ReadOnly)) {
            logError(tr("Couldn't open source file for reading."));
            return false;
        }

        inputContents = sourceFile.readAll();
        d->projectPath = QFileInfo(inputFile).path();
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(inputContents, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        logError(tr("Source file contains JSON errors."));
        return false;
    }

    d->rootObject = doc.object();

    if (!d->temporaryDir.isValid()) {
        logError(tr("Temporary directory could not be created."));
        return false;
    }

    QDir workingDir(QDir::currentPath());
    d->outputFile = workingDir.absoluteFilePath(parser.positionalArguments().at(1));

    if (parser.isSet("ffmpeg-command")) {
        d->ffmpegCommand = parser.value("ffmpeg-command");
    } else {
        d->ffmpegCommand = "ffmpeg";
    }

    if (parser.isSet("vcodec")) {
        d->vcodec = parser.value("vcodec");
    } else {
        d->vcodec = "libx264";
    }

    return true;
}

bool Renderer::constructElements() {
    d->viewport = new ViewportElement();
    d->viewport->setProperty("projectPath", d->projectPath);
    d->viewport->setProperty("requireThread", true);

    d->framerate = static_cast<uint>(d->rootObject.value("framerate").toInt());
    d->frameCount = d->rootObject.value("frameCount").toString().toULongLong();

    QJsonArray viewportSize = d->rootObject.value("viewportSize").toArray();
    d->viewport->setViewportSize(QSize(viewportSize.at(0).toInt(), viewportSize.at(1).toInt()));

    return d->viewport->load(d->rootObject.value("viewport").toObject());
}

void Renderer::beginRender() {
    logInfo(tr("Starting render job."));
    logInfo(tr("Writing temporary images into %1").arg(d->temporaryDir.path()));
    logInfo(tr("%n frames to export...", nullptr, static_cast<int>(d->frameCount)));

    QList<quint64> indices;
    indices.reserve(static_cast<int>(d->frameCount));
    for (quint64 i = 0; i < d->frameCount; i++) {
        indices.append(i);
    }

    QFuture<void> progress = QtConcurrent::map(indices, [ = ](const quint64 & frame) {
        QImage image(d->viewport->viewportSize(), QImage::Format_ARGB32);
        QPainter painter(&image);
        d->viewport->render(&painter, frame);
        bool success = image.save(d->temporaryDir.filePath(QStringLiteral("Frame%1.png").arg(frame, 5, 10, QChar('0'))), "PNG");
        if (!success) {
            logError(tr("Export frame %n failed", nullptr, static_cast<int>(frame)));
        }
    });

    QEventLoop* loop = new QEventLoop();

    QFutureWatcher<void> watcher;
    connect(&watcher, &QFutureWatcher<void>::progressValueChanged, this, [ = ](int progressValue) {
        logProgress(QString::number(progressValue) + "/" + QString::number(d->frameCount));
    });
    connect(this, &Renderer::stopProgress, &watcher, &QFutureWatcher<void>::cancel);
    connect(&watcher, &QFutureWatcher<void>::finished, loop, &QEventLoop::quit);
    watcher.setFuture(progress);
    loop->exec();
    delete loop;
}

bool Renderer::completeRender() {
    logInfo(tr("Frame output complete."));
    logInfo(tr("Calling on ffmpeg to complete the rendering work"));

    QProcess ffmpegProcess;
    ffmpegProcess.setProgram(d->ffmpegCommand);
    ffmpegProcess.setArguments({
        "-y",
        "-r", QString::number(d->framerate),
        "-s", QString::number(d->viewport->viewportSize().width()) + "x" + QString::number(d->viewport->viewportSize().height()),
        "-i", "Frame%05d.png",
        "-vcodec", d->vcodec,
        d->outputFile
    });
    ffmpegProcess.setWorkingDirectory(d->temporaryDir.path());
    ffmpegProcess.setProcessChannelMode(QProcess::ForwardedChannels);
    ffmpegProcess.start();
    ffmpegProcess.waitForFinished(-1);

    if (ffmpegProcess.exitCode() == 0) {
        logInfo(tr("Render job completed successfully."));
        return true;
    } else {
        logError(tr("ffmpeg couldn't render the video"));
        return false;
    }
}

void Renderer::stop() {
    d->stop = true;
    emit stopProgress();
}

bool Renderer::isStopped() {
    return d->stop;
}

void Renderer::logError(QString error) {
    QTextStream(stderr) << error << "\n";
}

void Renderer::logInfo(QString info) {
    QTextStream(stderr) << info << "\n";
}

void Renderer::logProgress(QString progress) {
    QTextStream(stdout) << progress << "\n";
}
