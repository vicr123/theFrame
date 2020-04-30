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
            Errored
        };

        explicit RenderJob(QByteArray projectFile, QString projectPath, QObject *parent = nullptr);
        ~RenderJob();

        void setOutputFileName(QString filename);
        QString outputFileName();

        QString jobDisplayName();

        void setFfmpegPath(QString ffmpeg);
        QString ffmpegPath();

        void setRenderer(QString renderer);
        QString renderer();

        void startRenderJob();
        State state();

        quint64 progress();
        quint64 maxProgress();

    signals:
        void outputFileNameChanged(QString filename);
        void ffmpegPathChanged(QString ffmpegPath);
        void rendererChanged(QString renderer);
        void stateChanged(State state);
        void progressChanged(quint64 progress, quint64 maxProgress);

    private:
        RenderJobPrivate* d;

};
typedef QSharedPointer<RenderJob> RenderJobPtr;

#endif // RENDERJOB_H
