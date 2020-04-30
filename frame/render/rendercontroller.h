#ifndef RENDERCONTROLLER_H
#define RENDERCONTROLLER_H

#include <QObject>
#include "renderjob.h"

struct RenderControllerPrivate;
class RenderController : public QObject
{
        Q_OBJECT
    public:
        explicit RenderController(QObject *parent = nullptr);

        static RenderController* instance();

        void queueRenderJob(RenderJobPtr job);
        QList<RenderJobPtr> jobs();

        bool haveUnfinishedJobs();
        void cancelAll();

    signals:

    private:
        RenderControllerPrivate* d;

        void attemptStartNextJob();
};

#endif // RENDERCONTROLLER_H
