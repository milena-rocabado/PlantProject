#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>

class AnalyzerManager;
class ROIWindow;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pbExaminarInput_clicked();

    void on_spnInitPos_valueChanged(int arg1);

    void on_spnEndPos_valueChanged(int arg1);

    void on_pbReset_clicked();

    void on_pbExaminarOutput_clicked();

    void on_pbProcess_clicked();

    void on_pbSetROI_clicked();

private:
    void disableElements_();

    void enableElements_();

    void resetSpnValues_();

    void setErrorState(QString errorMsg);

private:
    Ui::MainWindow *ui;

    std::shared_ptr<AnalyzerManager> manager_;
    std::unique_ptr<ROIWindow> roiWin_;
};

#endif // MAINWINDOW_H
