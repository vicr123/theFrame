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

        void shown();

    private slots:
        void on_browseForFileButton_clicked();

        void on_titleLabel_backButtonClicked();

        void on_browserForFfmpegButton_clicked();

        void on_browseForRenderer_clicked();

        void on_renderFile_textChanged(const QString &arg1);

        void on_ffmpegCommand_textChanged(const QString &arg1);

        void on_rendererPath_textEdited(const QString &arg1);

        void on_startRenderButton_clicked();

        void on_ffmpegTabWidget_currentChanged(int index);

        void on_downloadFfmpegButton_clicked();

        void on_removeFfmpegButton_clicked();

        void on_actionStart_Rendering_triggered();

        void on_actionStart_Rendering_changed();

        void on_leftList_currentRowChanged(int currentRow);

    signals:
        void renderingStarted(RenderJobPtr job);
        void done();

    private:
        Ui::RenderPopover *ui;
        RenderPopoverPrivate* d;

        void settingChanged(QString key, QVariant value);
        void ffmpegDownloadStateChanged();
        void ensureSettingsValid();

#ifdef Q_OS_MAC
        void setupMacOS();
#endif
};

#endif // RENDERPOPOVER_H
