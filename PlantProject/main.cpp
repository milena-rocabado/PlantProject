#include "mainwindow.h"
#include <QApplication>

#include "Segmentator.h"
#include <Test.h>
#include <AdaptiveModelSegmentator.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
