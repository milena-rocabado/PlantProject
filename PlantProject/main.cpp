#include "mainwindow.h"
#include <QApplication>
#include "analizador.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();

//    if (set_video("C:/Users/milena/git/PlantProject/Videos/climbing_bean_project3_leaf_folding.AVI")) {
//        // analizar_video();
//        analizar_2_frames();
//    }
//    return 0;
}
