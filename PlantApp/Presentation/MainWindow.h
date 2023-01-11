#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QEvent>
#include <memory>

#include "AnalyzerManager.h"

class ROIWindow;
class ConfigDisplayWindow;
class PotPositionWindow;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    inline static const std::string PROCESSING_STR { "Processing" };
    inline static const std::string PROCESSING_RGB { "rgb(255,255,0);" };
    inline static const std::string FINISHED_STR { "Finished" };
    inline static const std::string FINISHED_RGB { "rgb(85,255,0);" };

private slots:
    void on_lneInputPath_textChanged(const QString &text);

    void on_pbExaminarInput_clicked();

    void on_lneOutputPath_textChanged(const QString &text);

    void on_pbExaminarOutput_clicked();

    void on_pbCurrentConfig_clicked();

    void on_pbSetROI_clicked();

    void on_pbResetROI_clicked();

    void on_ProcessingDone_received();

    void on_pbSetPotPos_clicked();

    void on_spnInitPos_valueChanged(int arg1);

    void on_spnEndPos_valueChanged(int arg1);

    void on_pbResetPositions_clicked();

    void on_cbxDayNight_currentIndexChanged(int index);

    void on_chkOutput_stateChanged(int arg1);

    void on_pbProcess_clicked();

    void on_pbResetPotPos_clicked();

private:

    void launchProcessing_();

    void setEnabledConfigurationElements_(bool);

    void startProcessing_();

    void resetElements_();

    void resetSpnValues_();

    void setErrorState_(QString errorMsg);

    void setCbxValues_();

signals:
    void processingDone();

private:
    Ui::MainWindow *ui;

    std::shared_ptr<AnalyzerManager> manager_;
    std::unique_ptr<ROIWindow> roiWin_;
    std::unique_ptr<ConfigDisplayWindow> configDisplayWin_;
    std::unique_ptr<PotPositionWindow> potPositionWin_;

private:
    class ProcessingDoneEvent : public QEvent {
    public:
        ProcessingDoneEvent() :
            QEvent(static_cast<QEvent::Type>(QEvent::User + 1)) {}
        ~ProcessingDoneEvent();
    };
};

#endif // MAINWINDOW_H
