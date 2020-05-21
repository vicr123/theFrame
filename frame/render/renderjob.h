#ifndef RENDERJOB_H
#define RENDERJOB_H

#include <QObject>
#include <QSharedPointer>

struct RenderJobPrivate;
class RenderJob : public QObject
{
        Q_OBJECT
    public:
        enum State {
            Idle,
            Started,
            Finished,
            Errored,
            Cancelled
        };

        enum VideoCodec {
            MP4 = 0,
            WebM = 1
        };

        enum CodecCapability {
            Transparency = 0
        };

        explicit RenderJob(QByteArray projectFile, QString projectPath, QObject *parent = nullptr);
        ~RenderJob();

        void setOutputFileName(QString filename);
        QString outputFileName();

        void setVideoCodec(VideoCodec codec);
        VideoCodec videoCodec();
        QStringList videoCodecNameFilters();
        QList<CodecCapability> videoCodecCapabilities();

        void setCodecCapabilityEnabled(CodecCapability capability, bool enabled);
        bool codecCapabilityEnabled(CodecCapability capability);

        QString jobDisplayName();

        void enqueueRenderJob();
        void startRenderJob();
        void cancelRenderJob();
        State state();
        QByteArray processOutput();

        quint64 progress();
        quint64 maxProgress();

    signals:
        void outputFileNameChanged(QString filename);
        void stateChanged(State state);
        void progressChanged(quint64 progress, quint64 maxProgress);
        void videoCodecChanged(VideoCodec codec);
        void codecCapabilityChanged(CodecCapability capability, bool enabled);
        void outputAvailable(QByteArray output);

    private:
        RenderJobPrivate* d;

};
typedef QSharedPointer<RenderJob> RenderJobPtr;

#endif // RENDERJOB_H
