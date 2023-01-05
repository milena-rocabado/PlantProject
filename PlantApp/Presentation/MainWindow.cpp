#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>

#include <string>

#include "AnalyzerManager.h"
#include "Common.h"

#include "ROIWindow.h"
#include "ConfigDisplayWindow.h"

#include "Traces.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    manager_ = std::make_unique<AnalyzerManager>();
    roiWin_ = std::make_unique<ROIWindow>();
    configDisplayWin_ = std::make_unique<ConfigDisplayWindow>();

    ui->setupUi(this);

    ui->progressBar->setRange(0, 100);
    ui->progressBar->setValue(0);

    disableElements_();
    setCbxValues_();
}
//------------------------------------------------------------------------------
MainWindow::~MainWindow() {
    delete ui;
}
//------------------------------------------------------------------------------
void MainWindow::disableElements_() {
    ui->pbCurrentConfig->setEnabled(false);
    ui->pbSetROI->setEnabled(false);
    ui->pbResetROI->setEnabled(false);
    ui->spnInitPos->setEnabled(false);
    ui->spnEndPos->setEnabled(false);
    ui->pbResetPositions->setEnabled(false);
    ui->cbxDayNight->setEnabled(false);
    ui->lneOutputPath->setEnabled(false);
    ui->pbExaminarOutput->setEnabled(false);
    ui->chkOutput->setEnabled(false);
    ui->pbProcess->setEnabled(false);
}
//------------------------------------------------------------------------------
void MainWindow::resetSpnValues_() {
    int max = manager_->getMaxFrames();
    ui->spnInitPos->setRange(1, max);
    ui->spnEndPos->setRange(1, max);

    ui->spnInitPos->setValue(1);
    ui->spnEndPos->setValue(max);
}
//------------------------------------------------------------------------------
void MainWindow::setCbxValues_() {
    for (int i = 0; i < common::INTERVAL_NUM; i++) {
        std::stringstream ss;
        ss << static_cast<common::Interval>(i);
        ui->cbxDayNight->addItem(QString::fromStdString(ss.str()), i);
    }
}
//------------------------------------------------------------------------------
void MainWindow::resetElements_() {
    resetSpnValues_();

    ui->pbCurrentConfig->setEnabled(true);
    ui->pbSetROI->setEnabled(true);
    ui->pbResetROI->setEnabled(true);
    ui->spnInitPos->setEnabled(true);
    ui->spnEndPos->setEnabled(true);
    ui->pbResetPositions->setEnabled(true);
    ui->cbxDayNight->setEnabled(true);
    ui->lneOutputPath->setEnabled(true);
    ui->pbExaminarOutput->setEnabled(true);
    ui->chkOutput->setEnabled(true);
    ui->pbProcess->setEnabled(true);

    cv::Mat frame = manager_->getFrameFromVideo();
    roiWin_->setImage(frame);
    configDisplayWin_->setImage(frame);
}
//------------------------------------------------------------------------------
void MainWindow::setErrorState(QString errorMsg) {
    disableElements_();
    QMessageBox errorMsgBox;
    errorMsgBox.critical(this, "Error", errorMsg);
    errorMsgBox.setFixedSize(500,200);
}
//------------------------------------------------------------------------------
void MainWindow::updateProgress(double progress) {
    ui->progressBar->setValue(static_cast<int>(progress*100.));
}
//------------------------------------------------------------------------------
// SLOTS -----------------------------------------------------------------------
//------------------------------------------------------------------------------
void MainWindow::on_lneInputPath_textChanged(const QString &text) {
    TRACE("> MainWindow::on_lneInputPath_textChanged()");

    if (manager_->setInputPath(text.toStdString())) {
        resetElements_();
        ui->lneOutputPath->setText(QString::fromStdString(manager_->getOutputDirectory()));
    } else {
        QMessageBox errorMsgBox;
        errorMsgBox.critical(this, "Error", "Could not open input video");
        errorMsgBox.setFixedSize(500,200);
    }

    TRACE("< MainWindow::on_lneInputPath_textChanged()");
}
//------------------------------------------------------------------------------
void MainWindow::on_pbExaminarInput_clicked() {
    TRACE("> MainWindow::on_pbExaminarInput_clicked()");

    QString prevFp = ui->lneInputPath->text();
    QString fp = QFileDialog::getOpenFileName(this, "Select input video");

    if (fp.compare(prevFp) == 0) {
        on_lneInputPath_textChanged(fp);
    } else {
        ui->lneInputPath->setText(fp);
    }

    TRACE("< MainWindow::on_pbExaminarInput_clicked()");
}
//------------------------------------------------------------------------------
void MainWindow::on_pbCurrentConfig_clicked() {
    TRACE("> MainWindow::on_pbCurrentConfig_clicked()");
    configDisplayWin_->display(manager_->getROI(), manager_->getPotPosition());
    TRACE("< MainWindow::on_pbCurrentConfig_clicked()");
}
//------------------------------------------------------------------------------
void MainWindow::on_pbSetROI_clicked() {
    TRACE("> MainWindow::on_pbSetROI_clicked()");
    manager_->setROI(roiWin_->getROI());
    TRACE("< MainWindow::on_pbSetROI_clicked()");
}
//------------------------------------------------------------------------------
void MainWindow::on_pbResetROI_clicked() {
    manager_->resetROI();
}
//------------------------------------------------------------------------------
void MainWindow::on_spnInitPos_valueChanged(int pos) {
    manager_->setInitialPosition(pos);
}
//------------------------------------------------------------------------------
void MainWindow::on_spnEndPos_valueChanged(int pos) {
    manager_->setEndPosition(pos);
}
//------------------------------------------------------------------------------
void MainWindow::on_pbResetPositions_clicked() {
    resetSpnValues_();
}
//------------------------------------------------------------------------------
void MainWindow::on_pbExaminarOutput_clicked() {
    TRACE("> MainWindow::on_pbExaminarOutput_clicked()");

    QString fp = QFileDialog::getExistingDirectory(this, "Select output directory");
    ui->lneOutputPath->setText(fp);

    TRACE("< MainWindow::on_pbExaminarOutput_clicked()");
}
//------------------------------------------------------------------------------
void MainWindow::on_lneOutputPath_textChanged(const QString &text) {
    TRACE("> MainWindow::on_lneOutputPath_textChanged()");

    if (! manager_->setOutputDirectory(text.toStdString())) {
        setErrorState("Could not open output folder");
    }

    TRACE("> MainWindow::on_lneOutputPath_textChanged()");
}
//------------------------------------------------------------------------------
void MainWindow::on_chkOutput_stateChanged(int state) {
    manager_->setVideoOutputFlag(state == Qt::Checked);
}
//------------------------------------------------------------------------------
void MainWindow::on_pbProcess_clicked() {
    TRACE("> MainWindow::on_pbProcess_clicked()");

    manager_->setSubscriber(this);

    if (manager_->initialize()) {
        ui->pbProcess->setEnabled(false);
        manager_->launchProcessing();
        ui->pbProcess->setEnabled(true);
    } else {
        setErrorState(QString::fromStdString("Could not open output file " +
                      manager_->getOutputFilename()));
    }

    TRACE("< MainWindow::on_pbProcess_clicked()");
}
