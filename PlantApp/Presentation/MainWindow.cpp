#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>

#include "AnalyzerManager.h"

#include "ROIWindow.h"

#include "Traces.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    manager_ = std::make_unique<AnalyzerManager>();
    roiWin_ = std::make_unique<ROIWindow>();
    ui->setupUi(this);
    disableElements_();
}
//------------------------------------------------------------------------------
MainWindow::~MainWindow()
{
    delete ui;
}
//------------------------------------------------------------------------------
void MainWindow::disableElements_() {
    ui->pbSetROI->setEnabled(false);
    ui->spnInitPos->setEnabled(false);
    ui->spnEndPos->setEnabled(false);
    ui->pbReset->setEnabled(false);
    ui->cbxDayNight->setEnabled(false);
    ui->lneOutputPath->setEnabled(false);
    ui->pbExaminarOutput->setEnabled(false);
    ui->chkOutput->setEnabled(false);
    ui->pbProcess->setEnabled(false);
}
//------------------------------------------------------------------------------
void MainWindow::resetSpnValues_() {
    int max = manager_->getMaxFrames();
    ui->spnInitPos->setRange(0, max);
    ui->spnEndPos->setRange(0, max);

    ui->spnInitPos->setValue(0);
    ui->spnEndPos->setValue(max);
}
//------------------------------------------------------------------------------
void MainWindow::enableElements_() {
    resetSpnValues_();

    ui->pbSetROI->setEnabled(true);
    ui->spnInitPos->setEnabled(true);
    ui->spnEndPos->setEnabled(true);
    ui->pbReset->setEnabled(true);
    ui->cbxDayNight->setEnabled(true);
    ui->lneOutputPath->setEnabled(true);
    ui->pbExaminarOutput->setEnabled(true);
    ui->chkOutput->setEnabled(true);
    ui->pbProcess->setEnabled(true);
}
//------------------------------------------------------------------------------
void MainWindow::setErrorState(QString errorMsg) {
    disableElements_();
    QMessageBox errorMsgBox;
    errorMsgBox.critical(this, "Error", errorMsg);
    errorMsgBox.setFixedSize(500,200);
}
//------------------------------------------------------------------------------
// SLOTS -----------------------------------------------------------------------
//------------------------------------------------------------------------------
void MainWindow::on_pbExaminarInput_clicked() {
    TRACE("> MainWindow::on_pbExaminarInput_clicked()");

    QString fp = QFileDialog::getOpenFileName(this, "Select input video");
    ui->lneInputPath->setText(fp);

    if (manager_->setInputPath(fp.toStdString())) {
        enableElements_();
        ui->lneOutputPath->setText(QString::fromStdString(manager_->getOutputDirectory()));
    } else {
        QMessageBox errorMsgBox;
        errorMsgBox.critical(this, "Error", "Could not open input video");
        errorMsgBox.setFixedSize(500,200);
    }

    TRACE("< MainWindow::on_pbExaminarInput_clicked()");
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
void MainWindow::on_pbReset_clicked() {
    resetSpnValues_();
}
//------------------------------------------------------------------------------
void MainWindow::on_pbExaminarOutput_clicked() {
    TRACE(">MainWindow::on_pbExaminarOutput_clicked()");

    QString fp = QFileDialog::getExistingDirectory(this, "Select output directory");
    ui->lneOutputPath->setText(fp);

    if (manager_->setInputPath(fp.toStdString())) {
        enableElements_();
    } else {
        setErrorState("Could not open input video");
    }

    TRACE("<MainWindow::on_pbExaminarOutput_clicked()");
}
//------------------------------------------------------------------------------
void MainWindow::on_pbProcess_clicked() {
    TRACE(">MainWindow::on_pbProcess_clicked()");

    if (manager_->initialize()) {
        manager_->run();
    } else {
        setErrorState(QString::fromStdString("Could not open output file " +
                      manager_->getOutputFilename()));
    }

    TRACE("<MainWindow::on_pbProcess_clicked()");
}
//------------------------------------------------------------------------------
void MainWindow::on_pbSetROI_clicked() {
    TRACE(">MainWindow::on_pbSetROI_clicked()");
    roiWin_->setImage(manager_->getFrameFromVideo());
    manager_->setROI(roiWin_->getROI());
    TRACE("<MainWindow::on_pbSetROI_clicked()");
}
