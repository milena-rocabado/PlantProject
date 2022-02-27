#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "Analizador.h"

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
    void on_pbEjecutar_clicked();
    void on_pbExaminar_clicked();
    void on_chkAdaptativo_stateChanged(int state);

private:
    Ui::MainWindow *ui;
    Analizador *a;
    QLocale *spanish;

    void initialize();
    void signalsAndSlots();
    void set_working_directory();
    bool set_file();
    void crear_analizador_frames();
    void crear_analizador_video();
    void crear_analizador_bgS();
};

#endif // MAINWINDOW_H
