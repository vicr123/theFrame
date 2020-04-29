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
#include <tmessagebox.h>
#include <taboutdialog.h>
#include "prerenderer.h"

#include <QUndoStack>

struct MainWindowPrivate {
    QTimer* playTimer;
    qint64 playStartTime;
    quint64 startFrame;

    QUndoStack* undoStack;
    Prerenderer* prerenderer;

    QString currentFile;
    ViewportElement* viewport;

    bool doClose = false;
};

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

    ui->timeline->setUndoStack(d->undoStack);
    ui->timeline->setPrerenderer(d->prerenderer);
    ui->timeline->setViewportElement(d->viewport);
    ui->propertiesWidget->setTimeline(ui->timeline);
    ui->propertiesWidget->setUndoStack(d->undoStack);

    ui->viewport->setFrame(0);

    d->playTimer = new QTimer(this);
    connect(d->playTimer, &QTimer::timeout, this, &MainWindow::updatePlayFrame);
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
    if (d->doClose) return;
    if (!d->undoStack->isClean()) {
        event->ignore();
        this->ensureDiscardChanges()->then([=] {
            d->doClose = true;
            this->close();
        });
    }
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
