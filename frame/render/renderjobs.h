#ifndef RENDERJOBS_H
#define RENDERJOBS_H

#include <QWidget>

namespace Ui {
    class RenderJobs;
}

class RenderJobs : public QWidget
{
        Q_OBJECT

    public:
        explicit RenderJobs(QWidget *parent = nullptr);
        ~RenderJobs();

    signals:
        void done();

    private slots:
        void on_titleLabel_backButtonClicked();

        void on_jobsWidget_currentRowChanged(int currentRow);

    private:
        Ui::RenderJobs *ui;
};

#endif // RENDERJOBS_H
