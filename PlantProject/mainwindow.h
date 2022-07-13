#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "Segmentator.h"
#include "StaticModelSegmentator.h"

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
    void on_pbEjecutarSegm_clicked();
    void on_pbExaminar_clicked();

    void on_cboFijoConf_currentIndexChanged(int index);

    void on_chkDebug_stateChanged(int arg1);

    void on_pbEjecutar_clicked();

private:
    class PrivateData;

    Ui::MainWindow *ui;
    Segmentator *a;
    QLocale *spanish;
    PrivateData *p;

    void initialize();
    void signalsAndSlots();

    bool set_file();

    void init_tabGlobal();
    void init_tabSegmF();
    void init_tabSegmA();
    void init_tabBgS();

    void crear_analizadorGlobal();
    void crear_analizadorSegmF();
    void crear_analizadorSegmA();
    void crear_analizadorBgS();
};

#endif // MAINWINDOW_H
