#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QFileDialog>
#include <QMessageBox>

#include "AnalyzerManager.h"
#include "Traces.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}
//------------------------------------------------------------------------------
MainWindow::~MainWindow()
{
    delete ui;
}
//------------------------------------------------------------------------------
void MainWindow::initialize_() {
    ui->pbSetROI->setEnabled(false);
    ui->pbProcess->setEnabled(false);
}
//------------------------------------------------------------------------------
void MainWindow::on_pbExaminar_clicked() {
    TRACE(">MainWindow::on_pbExaminar_clicked");
    QString fp = QFileDialog::getOpenFileName(this, "Seleccionar video");
    ui->lnePath->setText(fp);

    if (manager_->setInputPath(fp.toStdString())) {
        ui->pbSetROI->setEnabled(true);
        ui->pbProcess->setEnabled(true);
    } else {
        QMessageBox errorMsgBox;
        errorMsgBox.critical(this ,"Error", "Could not open input video");
        errorMsgBox.setFixedSize(500,200);
    }

    TRACE("<MainWindow::on_pbExaminar_clicked");
}
//------------------------------------------------------------------------------
