#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>

#include <string>
#include <thread>

#include "AnalyzerManager.h"
#include "Common.h"

#include "ROIWindow.h"
#include "ConfigDisplayWindow.h"
#include "PotPositionWindow.h"

#include "Traces.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    manager_ = std::make_unique<AnalyzerManager>();
    roiWin_ = std::make_unique<ROIWindow>();
    configDisplayWin_ = std::make_unique<ConfigDisplayWindow>();
    potPositionWin_ = std::make_unique<PotPositionWindow>();

    ui->setupUi(this);

    setEnabledConfigurationElements_(false);
    setCbxValues_();

    connect(this, SIGNAL(processingDone()), this,
            SLOT(on_ProcessingDone_received()));
}
//------------------------------------------------------------------------------
MainWindow::~MainWindow() {
    delete ui;
}
//------------------------------------------------------------------------------
void MainWindow::setEnabledConfigurationElements_(bool enabled) {
    ui->pbCurrentConfig->setEnabled(enabled);
    ui->pbSetROI->setEnabled(enabled);
    ui->pbResetROI->setEnabled(enabled);
    ui->pbSetPotPos->setEnabled(enabled);
    ui->pbResetPotPos->setEnabled(enabled);
    ui->spnInitPos->setEnabled(enabled);
    ui->spnEndPos->setEnabled(enabled);
    ui->pbResetPositions->setEnabled(enabled);
    ui->cbxDayNight->setEnabled(enabled);
    ui->lneOutputPath->setEnabled(enabled);
    ui->pbExaminarOutput->setEnabled(enabled);
    ui->chkOutput->setEnabled(enabled);
    ui->pbProcess->setEnabled(enabled);
}
//------------------------------------------------------------------------------
void MainWindow::startProcessing_() {
    // Disable UI
    ui->lneInputPath->setEnabled(false);
    ui->pbExaminarInput->setEnabled(false);
    setEnabledConfigurationElements_(false);
    // Set indicator color
    ui->statusIndicator->setStyleSheet(QString::fromStdString("color: black;\nbackground-color:"+PROCESSING_RGB));
    // Set indicator text
    ui->statusIndicator->setText(QString::fromStdString(PROCESSING_STR));
    // Update indicator
    ui->statusIndicator->update();
    // Run in new thread
    std::thread thr(&MainWindow::launchProcessing_, this);
    thr.detach();
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

    cv::Mat frame = manager_->getFrameFromVideo();
    roiWin_->setImage(frame);
    configDisplayWin_->setImage(frame);
    potPositionWin_->setImage(frame);

    ui->lneOutputPath->setText(QString::fromStdString(manager_->getOutputDirectory()));
}
//------------------------------------------------------------------------------
void MainWindow::launchProcessing_() {
    // Run
    this->manager_->run();
    // Emit signal when done
    this->processingDone();
}
//------------------------------------------------------------------------------
// SLOTS -----------------------------------------------------------------------
//------------------------------------------------------------------------------
void MainWindow::on_lneInputPath_textChanged(const QString &text) {
    TRACE("> MainWindow::on_lneInputPath_textChanged()");

    AnalyzerManager::SetInputRetValues retval = manager_->setInputPath(text.toStdString());
    if (retval == AnalyzerManager::INPUT_SET) {
        setEnabledConfigurationElements_(true);
        resetElements_();
    } else {
        // Show error popup
        QString message;

        switch (retval) {
        case AnalyzerManager::PATH_NOT_VALID:
            message = "Provided path not valid"; break;
        case AnalyzerManager::VIDEO_NOT_OPEN:
            message = "Unable to open input video"; break;
        default:
            message = "Unknown error";
        }

        QMessageBox errorMsgBox;
        errorMsgBox.critical(this, "Error", message);
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
        // Beacause text is the same, slot not called automatically -> explicit
        // call
        on_lneInputPath_textChanged(fp);
    } else {
        ui->lneInputPath->setText(fp);
    }

    TRACE("< MainWindow::on_pbExaminarInput_clicked()");
}
//------------------------------------------------------------------------------
void MainWindow::on_lneOutputPath_textChanged(const QString &text) {
    TRACE("> MainWindow::on_lneOutputPath_textChanged()");

    AnalyzerManager::SetOutputRetValues retval = manager_->setOutputDirectory(text.toStdString());

    if (retval == AnalyzerManager::OUTPUT_SET) {
        return;
    }

    QString message;
    switch(retval) {
    case AnalyzerManager::OUTPATH_NOENT:
        message = "Output folder does not exist"; break;
    case AnalyzerManager::OUTPATH_NOACCESS:
        message = "Can't access output folder"; break;
    default:
        message = "Unknown error opening output folder"; break;
    }

    QMessageBox errorMsgBox;
    errorMsgBox.critical(this, "Error", message);
    errorMsgBox.setFixedSize(500,200);

    TRACE("> MainWindow::on_lneOutputPath_textChanged()");
}
//------------------------------------------------------------------------------
void MainWindow::on_pbExaminarOutput_clicked() {
    TRACE("> MainWindow::on_pbExaminarOutput_clicked()");

    QString prevPath = ui->lneOutputPath->text();
    QString path = QFileDialog::getExistingDirectory(this, "Select output directory");

    if (path.compare(prevPath) == 0) {
        on_lneOutputPath_textChanged(path);
    } else {
        ui->lneOutputPath->setText(path);
    }

    TRACE("< MainWindow::on_pbExaminarOutput_clicked()");
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
    TRACE("> MainWindow::on_pbResetROI_clicked()");

    manager_->resetROI();

    TRACE("< MainWindow::on_pbResetROI_clicked()");
}
//------------------------------------------------------------------------------
void MainWindow::on_pbSetPotPos_clicked() {
    TRACE("> MainWindow::on_pbSetPotPos_clicked()");

    manager_->setPotPosition(potPositionWin_->getPotPosition());

    TRACE("< MainWindow::on_pbSetPotPos_clicked()");
}
//------------------------------------------------------------------------------
void MainWindow::on_pbResetPotPos_clicked() {
    TRACE("> MainWindow::on_pbResetPotPos_clicked()");

    manager_->resetPotPosition();

    TRACE("< MainWindow::on_pbResetPotPos_clicked()");
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
void MainWindow::on_cbxDayNight_currentIndexChanged(int index) {
    manager_->setStartInterval(static_cast<common::Interval>(index));
}
//------------------------------------------------------------------------------
void MainWindow::on_chkOutput_stateChanged(int state) {
    manager_->setVideoOutputFlag(state == Qt::Checked);
}
//------------------------------------------------------------------------------
void MainWindow::on_pbProcess_clicked() {
    TRACE("> MainWindow::on_pbProcess_clicked()");

    if (ui->spnInitPos->value() > ui->spnEndPos->value()) {
        QMessageBox errorMsgBox;
        errorMsgBox.critical(this, "Error", "End position must be greater or"
                                            " equal initial position");
        errorMsgBox.setFixedSize(500,200);
        return;
    }

    AnalyzerManager::InitializeRetValues retval = manager_->initialize();
    if (retval == AnalyzerManager::INITIALIZED) {
        startProcessing_();
        QCoreApplication::processEvents();
    } else if (retval == AnalyzerManager::OUTFILE_ERROR) {
        QMessageBox errorMsgBox;
        errorMsgBox.critical(this, "Error",
                             QString::fromStdString("Could not open output file " +
                                                    manager_->getOutputFilename()));
        errorMsgBox.setFixedSize(500,200);
    }

    TRACE("< MainWindow::on_pbProcess_clicked()");
}
//------------------------------------------------------------------------------
void MainWindow::on_ProcessingDone_received() {
    // Set indicator color
    ui->statusIndicator->setStyleSheet(QString::fromStdString("color: black;\nbackground-color:"+FINISHED_RGB));
    // Set indicator text
    ui->statusIndicator->setText(QString::fromStdString(FINISHED_STR));
    // Update indicator
    ui->statusIndicator->update();

    ui->lneInputPath->setEnabled(true);
    ui->pbExaminarInput->setEnabled(true);
    setEnabledConfigurationElements_(true);
}


