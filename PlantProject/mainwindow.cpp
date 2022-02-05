#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include "analizador.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_loadButton_clicked()
{
    QString filepath = "C:/Users/milena/git/PlantProject/Videos/climbing_bean_project3_leaf_folding.AVI";
    //QFileDialog::getOpenFileName();
    if (set_video(filepath.toLatin1().data())) {
        // analizar_video();
        analizar_2_frames();
    }
}
