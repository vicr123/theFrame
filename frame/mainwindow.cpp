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
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <elements/timelineelement.h>
#include <elements/rectangleelement.h>
#include <QFileDialog>
#include <QMessageBox>
#include <QCloseEvent>
#include <QDesktopServices>
#include <tsettings.h>
#include <tmessagebox.h>
#include <taboutdialog.h>
#include "prerenderer.h"
#include <tpopover.h>
#include <ttoast.h>

#include "render/renderpopover.h"
#include "render/renderjobs.h"
#include "render/rendercontroller.h"

#include <QUndoStack>

struct MainWindowPrivate {
    QTimer* playTimer;
    qint64 playStartTime;
    quint64 startFrame;

    QUndoStack* undoStack;
    Prerenderer* prerenderer;

    QString currentFile;
    ViewportElement* viewport;

    tSettings settings;

    uint closeFlag = 0;
    bool shouldSaveWindowState = false;
};

QList<MainWindow*> MainWindow::openWindows = QList<MainWindow*>();

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {
    ui->setupUi(this);
    d = new MainWindowPrivate();

    d->undoStack = new QUndoStack(this);
    QAction* undoAction = d->undoStack->createUndoAction(this, tr("Undo"));
    QAction* redoAction = d->undoStack->createRedoAction(this, tr("Redo"));
    undoAction->setIcon(QIcon::fromTheme("edit-undo"));
    redoAction->setIcon(QIcon::fromTheme("edit-redo"));
    undoAction->setShortcut(tr("CTRL+Z"));
    redoAction->setShortcut(tr("CTRL+SHIFT+Z"));
    ui->menuEdit->insertAction(ui->actionDeleteTransition, undoAction);
    ui->menuEdit->insertAction(ui->actionDeleteTransition, redoAction);
    ui->menuEdit->insertSeparator(ui->actionDeleteTransition);
    connect(d->undoStack, &QUndoStack::cleanChanged, this, [=](bool clean) {
        this->setWindowModified(!clean);
    });

    d->prerenderer = new Prerenderer();
    d->viewport = ui->viewport->rootElement();
    d->prerenderer->setViewportElement(d->viewport);
    d->prerenderer->setTimeline(ui->timeline);

    ui->viewport->setPrerenderer(d->prerenderer);
    ui->viewport->setTimeline(ui->timeline);

    ui->timeline->setUndoStack(d->undoStack);
    ui->timeline->setPrerenderer(d->prerenderer);
    ui->timeline->setViewportElement(d->viewport);
    ui->propertiesWidget->setTimeline(ui->timeline);
    ui->propertiesWidget->setUndoStack(d->undoStack);

    ui->menuWindow->addAction(ui->timelineDockWidget->toggleViewAction());
    ui->menuWindow->addAction(ui->propertiesDockWidget->toggleViewAction());

    ui->viewport->setFrame(0);

    d->playTimer = new QTimer(this);
    connect(d->playTimer, &QTimer::timeout, this, &MainWindow::updatePlayFrame);

    ui->stackedWidget->setCurrentWidget(ui->welcomePage);
    ui->stackedWidget->setCurrentAnimation(tStackedWidget::Fade);

    ui->welcomePage->installEventFilter(this);
    ui->newProjectPage->installEventFilter(this);

    QFont titleFont = ui->welcomeTitle->font();
#ifdef Q_OS_MAC
    titleFont.setPointSize(30);
#else
#endif
    ui->welcomeTitle->setFont(titleFont);
    ui->newProjectTitle->setFont(titleFont);
}

MainWindow::~MainWindow() {
    delete d;
    delete ui;
}


void MainWindow::on_timeline_currentFrameChanged(quint64 frame) {
    ui->viewport->setFrame(frame);
}

void MainWindow::on_actionNextFrame_triggered() {
    ui->timeline->setCurrentFrame(ui->timeline->currentFrame() + 1);
}

void MainWindow::on_actionPreviousFrame_triggered() {
    quint64 frame = ui->timeline->currentFrame();
    if (frame != 0) frame--;
    ui->timeline->setCurrentFrame(frame);
}

void MainWindow::on_actionDeleteTransition_triggered() {
    ui->timeline->deleteSelected();
}

void MainWindow::on_actionZoomIn_triggered() {
    double spacing = ui->timeline->frameSpacing();
    spacing *= 1.25;
    ui->timeline->setFrameSpacing(spacing);
}

void MainWindow::on_actionZoomOut_triggered() {
    double spacing = ui->timeline->frameSpacing();
    spacing *= 0.8;
    ui->timeline->setFrameSpacing(spacing);
}

void MainWindow::on_playButton_toggled(bool checked) {
    if (checked) {
        if (ui->timeline->currentFrame() == ui->timeline->frameCount() - 1) {
            d->startFrame = 0;
        } else {
            d->startFrame = ui->timeline->currentFrame();
        }
        d->playStartTime = QDateTime::currentMSecsSinceEpoch();
        d->playTimer->start();
    } else {
        d->playTimer->stop();
    }
}

void MainWindow::updatePlayFrame() {
    qint64 msecsAfterFrame = QDateTime::currentMSecsSinceEpoch() - d->playStartTime;
    qint64 msecsPerFrame = 1000 / ui->timeline->framerate();

    qint64 advancedFrames = msecsAfterFrame / msecsPerFrame;
    quint64 newFrame = d->startFrame + static_cast<quint64>(advancedFrames);

    if (newFrame > ui->timeline->frameCount()) {
        ui->timeline->setCurrentFrame(ui->timeline->frameCount() - 1);
        ui->playButton->setChecked(false);
    } else {
        ui->timeline->setCurrentFrame(newFrame);
    }
}

void MainWindow::on_actionPlay_triggered() {
    ui->playButton->toggle();
}

void MainWindow::on_actionFirstFrame_triggered() {
    ui->playButton->setChecked(false);
    ui->timeline->setCurrentFrame(0);
}

void MainWindow::on_actionLastFrame_triggered() {
    ui->playButton->setChecked(false);
    ui->timeline->setCurrentFrame(ui->timeline->frameCount() - 1);
}

void MainWindow::on_firstFrameButton_clicked() {
    ui->actionFirstFrame->trigger();
}

void MainWindow::on_lastFrameButton_clicked() {
    ui->actionLastFrame->trigger();
}

void MainWindow::on_actionSave_triggered() {
    this->save();
}

void MainWindow::on_actionSaveAs_triggered() {
    this->saveAs();
}

void MainWindow::on_actionOpen_triggered() {
    QFileDialog* fileDialog = new QFileDialog(this);
    fileDialog->setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog->setNameFilters({tr("theFrame Project Files (*.tfrproj)")});
    fileDialog->setWindowFlag(Qt::Sheet);
    fileDialog->setWindowModality(Qt::WindowModal);
    connect(fileDialog, &QFileDialog::finished, this, [ = ](int result) {
        if (result == QFileDialog::Accepted) {
            auto loadFile = [=] {
                //Attempt to load this file
                QFile file(fileDialog->selectedFiles().first());
                file.open(QSaveFile::ReadOnly);

                QJsonParseError error;
                QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);

                if (error.error != QJsonParseError::NoError || !doc.isObject()) {
                    //Error Error!
                    return;
                }

                if (!ui->timeline->load(doc.object())) {
                    //Error Error!
                    return;
                }

                d->currentFile = fileDialog->selectedFiles().first();
                this->setWindowFilePath(d->currentFile);
                QString projectPath = QFileInfo(d->currentFile).path();
                d->viewport->setProperty("projectPath", projectPath);
                ui->propertiesWidget->setProjectPath(projectPath);

                d->undoStack->clear();
                ui->stackedWidget->setCurrentWidget(ui->mainPage);
            };

            if (!d->undoStack->isClean()) {
                this->ensureDiscardChanges()->then(loadFile)->error([=](QString reason) {
                    fileDialog->deleteLater();
                });
            } else {
                loadFile();
            }
        } else {
            fileDialog->deleteLater();
        }
    });
    fileDialog->open();
}

void MainWindow::on_actionAbout_triggered() {
    tAboutDialog dialog;
    dialog.exec();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if ((d->closeFlag & 1) == 0) {
        if (!d->undoStack->isClean()) {
            event->ignore();
            this->ensureDiscardChanges()->then([=] {
                d->closeFlag |= 1;
                this->close();
            })->error([=](QString error) {
                //Don't quit the app
                d->closeFlag = 0;
    #ifdef Q_OS_MAC
                QApplication::setQuitOnLastWindowClosed(false);
    #endif
            });
        }
    }

    if ((d->closeFlag & 2) == 0 && openWindows.count() == 1 && RenderController::instance()->haveUnfinishedJobs()
        #ifdef Q_OS_MAC
            && QApplication::quitOnLastWindowClosed()
        #endif
            ) {
        event->ignore();

        tMessageBox* box = new tMessageBox(this);
        box->setWindowTitle(tr("Running Render Jobs"));
        box->setText(tr("You have running render jobs. If you exit theFrame, they'll be cancelled"));
        box->setIcon(tMessageBox::Warning);
        QPushButton* button = box->addButton(tr("Exit Anyway"), tMessageBox::DestructiveRole);
        box->setStandardButtons(tMessageBox::Cancel);
        box->setWindowModality(Qt::WindowModal);
        box->setWindowFlag(Qt::Sheet);
        connect(button, &QPushButton::clicked, this, [=] {
            RenderController::instance()->cancelAll();
            d->closeFlag |= 2;
            this->close();
        });
        connect(box, &tMessageBox::finished, this, [=](int result) {
            if (result == tMessageBox::Cancel) {
                //Don't quit the app
                d->closeFlag = 0;
#ifdef Q_OS_MAC
            QApplication::setQuitOnLastWindowClosed(false);
#endif
            }
            box->deleteLater();
        });
        box->open();
    }

    if (d->shouldSaveWindowState) d->settings.setValue("Window/WindowState", this->saveState());
}

bool MainWindow::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == ui->welcomePage || watched == ui->newProjectPage) {
        if (event->type() == QEvent::Paint) {
            QPainter painter(static_cast<QWidget*>(watched));
            QLinearGradient grad;
            grad.setColorAt(0, QColor(255, 200, 200));
            grad.setColorAt(1, QColor(255, 100, 100));
            grad.setStart(ui->welcomePage->layout()->contentsMargins().left() - 50, 0);
            grad.setFinalStop(0, this->height());
            painter.setPen(Qt::transparent);
            painter.setBrush(grad);
            painter.drawRect(0, 0, ui->welcomePage->layout()->contentsMargins().left() - 50, this->height());
        }
    }
    return false;
}

tPromise<void>* MainWindow::save()
{
    return tPromise<void>::runOnSameThread([=](tPromiseFunctions<void>::SuccessFunction res, tPromiseFunctions<void>::FailureFunction rej) {
        if (d->currentFile.isEmpty()) {
            this->saveAs()->then(res)->error(rej);
            return;
        }

        QSaveFile saveFile(d->currentFile);
        saveFile.open(QSaveFile::WriteOnly);

        QJsonDocument doc(ui->timeline->save());
        saveFile.write(doc.toJson(QJsonDocument::Compact));
        if (!saveFile.commit()) {
            //Error Error!
            QMessageBox::critical(this, tr("Error"), tr("Sorry, we couldn't save the file. Check that there is enough disk space and that you have permission to write to the file.\n\nDon't close the window until you've managed to save your changes, otherwise you may lose data."));
            rej("Save Error");
        } else {
            d->undoStack->setClean();
            res();
        }
    });
}

tPromise<void>* MainWindow::saveAs()
{
    return tPromise<void>::runOnSameThread([=](tPromiseFunctions<void>::SuccessFunction res, tPromiseFunctions<void>::FailureFunction rej) {
        QFileDialog* fileDialog = new QFileDialog(this);
        fileDialog->setAcceptMode(QFileDialog::AcceptSave);
        fileDialog->setNameFilters({tr("theFrame Project Files (*.tfrproj)")});
        fileDialog->setWindowFlag(Qt::Sheet);
        fileDialog->setWindowModality(Qt::WindowModal);
        connect(fileDialog, &QFileDialog::finished, this, [ = ](int result) {
            if (result == QFileDialog::Accepted) {
                d->currentFile = fileDialog->selectedFiles().first();
                this->setWindowFilePath(d->currentFile);
                QString projectPath = QFileInfo(d->currentFile).path();
                d->viewport->setProperty("projectPath", projectPath);
                ui->propertiesWidget->setProjectPath(projectPath);

                this->save()->then(res)->error(rej);
            } else {
                rej("Cancelled");
            }
            fileDialog->deleteLater();
        });
        fileDialog->open();
    });
}

tPromise<void>* MainWindow::ensureDiscardChanges()
{
    return tPromise<void>::runOnSameThread([=](tPromiseFunctions<void>::SuccessFunction res, tPromiseFunctions<void>::FailureFunction rej) {
        tMessageBox* box = new tMessageBox(this);
        box->setWindowTitle(tr("Save Changes?"));
        box->setText(tr("You'll lose any unsaved changes in this project if you don't save."));
        box->setIcon(tMessageBox::Warning);
        box->setStandardButtons(tMessageBox::Save | tMessageBox::Discard | tMessageBox::Cancel);
        box->setWindowModality(Qt::WindowModal);
        box->setWindowFlag(Qt::Sheet);
        connect(box, &tMessageBox::finished, this, [=](int result) {
            if (result == tMessageBox::Save) {
                this->save()->then([=] {
                    res();
                })->error(rej);
            } else if (result == tMessageBox::Discard) {
                res();
            } else {
                rej("Cancelled");
            }
            box->deleteLater();
        });
        box->open();
    });
}

void MainWindow::on_actionSet_In_Point_triggered()
{
    ui->timeline->setInPoint(ui->timeline->currentFrame());
}

void MainWindow::on_actionSet_Out_Point_triggered()
{
    ui->timeline->setOutPoint(ui->timeline->currentFrame());
}

void MainWindow::on_actionClear_In_Out_Points_triggered()
{
    ui->timeline->clearInOutPoint();
}

void MainWindow::on_actionEnable_Prerendering_toggled(bool arg1)
{
    d->prerenderer->setEnablePrerendering(arg1);
}

void MainWindow::on_actionSources_triggered()
{
    QDesktopServices::openUrl(QUrl("https://github.com/vicr123/theFrame"));
}

void MainWindow::on_actionFile_Bug_triggered()
{
    QDesktopServices::openUrl(QUrl("https://github.com/vicr123/theFrame/issues"));
}

void MainWindow::on_newProjectButton_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->newProjectPage);
}

void MainWindow::on_doCreateButton_clicked()
{
    d->viewport->setViewportSize(QSize(ui->createResXBox->value(), ui->createResYBox->value()));
    ui->timeline->setFramerate(ui->createFpsBox->value());

    this->saveAs()->then([=] {
        ui->stackedWidget->setCurrentWidget(ui->mainPage);
    });
}

void MainWindow::on_openProjectButton_clicked()
{
    ui->actionOpen->trigger();
}

void MainWindow::on_stackedWidget_currentChanged(int arg1)
{
    bool menuState;
    if (ui->stackedWidget->currentWidget() == ui->mainPage) {
        menuState = true;
        d->shouldSaveWindowState = true;

        if (d->settings.contains("Window/WindowState")) {
            this->restoreState(d->settings.value("Window/WindowState").toByteArray());
        } else {
            ui->timelineDockWidget->setVisible(true);
            ui->propertiesDockWidget->setVisible(true);
        }
    } else {
        menuState = false;
        d->shouldSaveWindowState = false;

        d->settings.setValue("Window/WindowState", this->saveState());
        d->settings.sync();

        ui->timelineDockWidget->setVisible(false);
        ui->propertiesDockWidget->setVisible(false);
    }

    ui->menubar->setVisible(menuState);
    ui->menuEdit->setEnabled(menuState);
    ui->menuView->setEnabled(menuState);
    ui->menuTimeline->setEnabled(menuState);
    ui->menuWindow->setEnabled(menuState);
    ui->actionSave->setEnabled(menuState);
    ui->actionSaveAs->setEnabled(menuState);
    ui->actionRender->setEnabled(menuState);
}

void MainWindow::on_actionExit_triggered()
{
#ifdef Q_OS_MAC
    QApplication::setQuitOnLastWindowClosed(true);
#endif
    QApplication::closeAllWindows();
}

void MainWindow::on_actionClose_triggered()
{
    this->close();
}

void MainWindow::on_actionNew_triggered()
{
    auto newFile = [=] {
        ui->stackedWidget->setCurrentWidget(ui->newProjectPage);
        d->viewport->clearChildren();
        d->viewport->clearTimelineElements();
        d->undoStack->clear();

        d->currentFile = "";
        this->setWindowFilePath("");
        QString projectPath = "";
        d->viewport->setProperty("projectPath", projectPath);
        ui->propertiesWidget->setProjectPath(projectPath);
    };

    if (!d->undoStack->isClean()) {
        this->ensureDiscardChanges()->then(newFile);
    } else {
        newFile();
    }
}

void MainWindow::on_newProjectBackButton_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->welcomePage);
}

void MainWindow::on_actionNew_Window_triggered()
{
    MainWindow* newWindow = new MainWindow();
    newWindow->show();
}

void MainWindow::showEvent(QShowEvent* event)
{
    Q_UNUSED(event);
    this->openWindows.append(this);
}

void MainWindow::hideEvent(QHideEvent* event)
{
    Q_UNUSED(event);
    this->openWindows.removeAll(this);
}

void MainWindow::on_actionRender_triggered()
{
    QJsonDocument doc(ui->timeline->save());

    RenderPopover* render = new RenderPopover(doc.toJson(QJsonDocument::Compact), QFileInfo(d->currentFile).path());
    tPopover* popover = new tPopover(render);
    popover->setPopoverWidth(SC_DPI(600));
    connect(render, &RenderPopover::renderingStarted, this, [=](RenderJobPtr job) {
        if (job->state() == RenderJob::Idle || job->state() == RenderJob::Started) {
            tToast* toast = new tToast();
            if (job->state() == RenderJob::Idle) {
                toast->setTitle(tr("Rendering Queued"));
                toast->setText(tr("%1 has been queued for rendering.").arg(job->jobDisplayName()));
            } else {
                toast->setTitle(tr("Rendering Started"));
                toast->setText(tr("%1 has started rendering.").arg(job->jobDisplayName()));
            }
            toast->setActions({
                                  {"viewJobs", tr("View Render Jobs")}
                              });
            connect(toast, &tToast::actionClicked, this, [=](QString key) {
                if (key == "viewJobs") ui->actionRender_Jobs->trigger();
            });
            connect(toast, &tToast::dismissed, toast, &tToast::deleteLater);
            toast->show(this);
        }
    });
    connect(render, &RenderPopover::done, popover, &tPopover::dismiss);
    connect(popover, &tPopover::dismissed, render, &RenderPopover::deleteLater);
    connect(popover, &tPopover::dismissed, popover, &tPopover::deleteLater);
    popover->show(this);
}

void MainWindow::on_actionRender_Jobs_triggered()
{
    RenderJobs* render = new RenderJobs();
    tPopover* popover = new tPopover(render);
    popover->setPopoverWidth(SC_DPI(-300));
    connect(render, &RenderJobs::done, popover, &tPopover::dismiss);
    connect(popover, &tPopover::dismissed, render, &RenderPopover::deleteLater);
    connect(popover, &tPopover::dismissed, popover, &tPopover::deleteLater);
    popover->show(this);
}
