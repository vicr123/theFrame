#ifndef FFMPEGDETECTOR_H
#define FFMPEGDETECTOR_H

#include <QObject>

struct FFmpegDetectorPrivate;
class FFmpegDetector : public QObject
{
        Q_OBJECT
    public:
        ~FFmpegDetector();

        static FFmpegDetector* instance();

        QString systemFfmpegPath();
        QString theframeFfmpegPath();
        QString renderFfmpegPath();
        bool systemFfmpegAvailable();
        bool theframeFfmpegAvailable();
        bool renderFfmpegAvailable();

        void downloadFfmpeg();
        bool isFfmpegDownloading();
        bool isFfmpegDownloadError();
        void removeFfmpeg();

        quint64 currentProgress();
        quint64 totalProgress();

    signals:
        void ffmpegDownloadingChanged(bool downloading);
        void ffmpegDownloadProgressChanged();

    private:
        explicit FFmpegDetector(QObject *parent = nullptr);
        FFmpegDetectorPrivate* d;

        void ffmpegDownloadErrored();
};

#endif // FFMPEGDETECTOR_H
