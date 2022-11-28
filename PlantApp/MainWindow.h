#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class AnalyzerManager;

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
    void on_pbExaminar_clicked();

private:
    void initialize_();
private:
    Ui::MainWindow *ui;
    AnalyzerManager *manager_;
};

#endif // MAINWINDOW_H
