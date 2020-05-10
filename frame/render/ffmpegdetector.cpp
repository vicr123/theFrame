#include "ffmpegdetector.h"

#include <the-libs_global.h>
#include <QDir>
#include <QStandardPaths>
#include <QSaveFile>

#include <QNetworkAccessManager>
#include <QNetworkReply>

#include <QJsonDocument>
#include <QJsonObject>

struct FFmpegDetectorPrivate {
    QNetworkAccessManager mgr;
    bool downloading = false;
    bool downloadError = false;

    quint64 current = 0;
    quint64 total = 0;
};

FFmpegDetector::FFmpegDetector(QObject *parent) : QObject(parent)
{
    d = new FFmpegDetectorPrivate();
}

void FFmpegDetector::ffmpegDownloadErrored()
{
    d->downloadError = true;
    d->downloading = false;
    emit ffmpegDownloadingChanged(false);
    return;
}

FFmpegDetector::~FFmpegDetector()
{
    delete d;
}

FFmpegDetector* FFmpegDetector::instance()
{
    static FFmpegDetector* i = new FFmpegDetector();
    return i;
}

QString FFmpegDetector::systemFfmpegPath()
{
    QString ffmpegExecutable = QStandardPaths::findExecutable("ffmpeg");
    if (!ffmpegExecutable.isEmpty()) return ffmpegExecutable;

    return "";
}

QString FFmpegDetector::theframeFfmpegPath()
{
#ifdef T_OS_UNIX_NOT_MAC
    return "/usr/bin/ffmpeg";
#else
    QString extension;
#ifdef Q_OS_WIN
    extension = ".exe";
#endif
    return QDir::cleanPath(QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation)).absoluteFilePath("ffmpeg" + extension));
#endif
}

QString FFmpegDetector::renderFfmpegPath()
{
    QString path = theframeFfmpegPath();
    if (QFile::exists(path)) return path;

    return systemFfmpegPath();
}

bool FFmpegDetector::systemFfmpegAvailable()
{
    return QFile(systemFfmpegPath()).exists();
}

bool FFmpegDetector::theframeFfmpegAvailable()
{
    return QFile(theframeFfmpegPath()).exists();
}

bool FFmpegDetector::renderFfmpegAvailable()
{
    return theframeFfmpegAvailable() || systemFfmpegAvailable();
}

void FFmpegDetector::downloadFfmpeg()
{
    if (d->downloading) return;
    d->downloading = true;
    d->downloadError = false;
    d->total = 0;
    d->current = 0;
    emit ffmpegDownloadingChanged(true);

    QString platform;
#if defined(Q_OS_MAC)
    platform = "mac";
#elif defined(Q_OS_WIN)
    platform = "win64";
#endif

    QNetworkRequest req(QUrl(QStringLiteral("https://packages.vicr123.com/ffmpeg/%1/meta.json").arg(platform)));
    QNetworkReply* metaReply = d->mgr.get(req);
    connect(metaReply, &QNetworkReply::finished, this, [=] {
        if (metaReply->error() != QNetworkReply::NoError) {
            //Bail out here
            ffmpegDownloadErrored();
            metaReply->deleteLater();
            return;
        }

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(metaReply->readAll(), &parseError);
        if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
            //Bail out here
            ffmpegDownloadErrored();
            metaReply->deleteLater();
            return;
        }

        QJsonObject obj = doc.object();
        if (!obj.contains("version") || !obj.contains("bin")) {
            //Bail out here
            ffmpegDownloadErrored();
            metaReply->deleteLater();
            return;
        }

        //Make sure the directory exists
        if (!QDir::root().mkpath(QFileInfo(theframeFfmpegPath()).path())) {
            //Bail out here
            ffmpegDownloadErrored();
            metaReply->deleteLater();
            return;
        }

        //Start downloading FFMpeg
        QSaveFile* ffmpegOutput = new QSaveFile(theframeFfmpegPath());
        if (!ffmpegOutput->open(QSaveFile::WriteOnly)) {
            //Bail out here
            ffmpegDownloadErrored();
            ffmpegOutput->deleteLater();
            metaReply->deleteLater();
            return;
        }

        QNetworkRequest req(QUrl(QStringLiteral("https://packages.vicr123.com/ffmpeg/%1/%2").arg(platform).arg(obj.value("bin").toString())));
        QNetworkReply* dataReply = d->mgr.get(req);
        connect(dataReply, &QNetworkReply::downloadProgress, this, [=](quint64 progress, quint64 total) {
            d->current = progress;
            d->total = total;
            emit ffmpegDownloadProgressChanged();
        });
        connect(dataReply, &QNetworkReply::readyRead, this, [=] {
            ffmpegOutput->write(dataReply->readAll());
        });
        connect(dataReply, &QNetworkReply::finished, this, [=] {
            if (dataReply->error() != QNetworkReply::NoError) {
                //Bail out here
                ffmpegOutput->cancelWriting();
                ffmpegDownloadErrored();
                ffmpegOutput->deleteLater();
                dataReply->deleteLater();
                return;
            }

#ifdef Q_OS_UNIX
            QSaveFile::Permissions permissions = ffmpegOutput->permissions();
            permissions |= QSaveFile::ExeOwner;
            if (!ffmpegOutput->setPermissions(permissions)) {
                //Bail out here
                ffmpegDownloadErrored();
                ffmpegOutput->deleteLater();
                dataReply->deleteLater();
                return;
            }
#endif

            if (!ffmpegOutput->commit()) {
                //Bail out here
                ffmpegDownloadErrored();
                ffmpegOutput->deleteLater();
                dataReply->deleteLater();
                return;
            }

            //Download was successful
            d->downloadError = false;
            d->downloading = false;
            emit ffmpegDownloadingChanged(false);

            ffmpegOutput->deleteLater();
            dataReply->deleteLater();
        });

        metaReply->deleteLater();
    });
}

bool FFmpegDetector::isFfmpegDownloading()
{
    return d->downloading;
}

bool FFmpegDetector::isFfmpegDownloadError()
{
    return d->downloadError;
}

void FFmpegDetector::removeFfmpeg()
{
    if (theframeFfmpegAvailable()) {
        QFile::remove(theframeFfmpegPath());
    }
}

quint64 FFmpegDetector::currentProgress()
{
    return d->current;
}

quint64 FFmpegDetector::totalProgress()
{
    return d->total;
}
