#include "mainwindow.h"
#include <QApplication>

#include <TestSegmentator.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

//    test_analyzer();

    return a.exec();
}
