#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>

#include "AnalyzerManager.h"

class ROIWindow;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow, AnalyzerManager::Subscriber
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void on_pbExaminarInput_clicked();

    void on_spnInitPos_valueChanged(int arg1);

    void on_spnEndPos_valueChanged(int arg1);

    void on_pbReset_clicked();

    void on_pbExaminarOutput_clicked();

    void on_pbProcess_clicked();

    void on_pbSetROI_clicked();

    void on_chkOutput_stateChanged(int arg1);

private:
    void disableElements_();

    void enableElements_();

    void resetSpnValues_();

    void setErrorState(QString errorMsg);

    void updateProgress(double progress) override;

private:
    Ui::MainWindow *ui;

    std::shared_ptr<AnalyzerManager> manager_;
    std::unique_ptr<ROIWindow> roiWin_;
};

#endif // MAINWINDOW_H
