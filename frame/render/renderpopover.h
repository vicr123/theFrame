#ifndef RENDERPOPOVER_H
#define RENDERPOPOVER_H

#include <QWidget>
#include "renderjob.h"

namespace Ui {
    class RenderPopover;
}

struct RenderPopoverPrivate;
class RenderPopover : public QWidget
{
        Q_OBJECT

    public:
        explicit RenderPopover(QByteArray projectFile, QString projectPath, QWidget *parent = nullptr);
        ~RenderPopover();

    private slots:
        void on_browseForFileButton_clicked();

        void on_titleLabel_backButtonClicked();

        void on_browserForFfmpegButton_clicked();

        void on_browseForRenderer_clicked();

        void on_renderFile_textChanged(const QString &arg1);

        void on_ffmpegCommand_textChanged(const QString &arg1);

        void on_rendererPath_textEdited(const QString &arg1);

        void on_startRenderButton_clicked();

    signals:
        void renderingStarted(RenderJobPtr job);
        void done();

    private:
        Ui::RenderPopover *ui;
        RenderPopoverPrivate* d;
};

#endif // RENDERPOPOVER_H
