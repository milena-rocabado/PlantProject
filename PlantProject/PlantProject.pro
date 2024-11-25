#-------------------------------------------------
#
# Project created by QtCreator 2022-01-26T16:27:03
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = PlantProject
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS \
        DEBUG

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

DEFINES += DUMP_ON

SOURCES += \
        Segmentator/AdaptiveModelSegmentator.cpp \
        Segmentator/BgSubtractorSegmentator.cpp \
        Segmentator/GlobalSegmentator.cpp \
        Segmentator/Segmentator.cpp \
        Segmentator/StaticModelSegmentator.cpp \
        Analyzer/Analyzer.cpp \
        Calculator/Calculator.cpp \
        Test/TestSegmentator.cpp \
        Utils/Utils.cpp \
        main.cpp \
        mainwindow.cpp

HEADERS += \
        Segmentator/AdaptiveModelSegmentator.h \
        Segmentator/BgSubtractorSegmentator.h \
        Segmentator/GlobalSegmentator.h \
        Segmentator/Segmentator.h \
        Segmentator/StaticModelSegmentator.h \
        Analyzer/Analyzer.h \
        Calculator/Calculator.h \
        Test/TestSegmentator.h \
        Utils/Utils.h \
        mainwindow.h

FORMS += \
        mainwindow.ui

INCLUDEPATH += "C:\OpenCV\OpenCV4.1.1G\include" \
               ./Segmentator \
               ./Analyzer \
               ./Test \
               ./Calculator \
               ./Utils

LIBS += -L"C:\OpenCV\OpenCV4.1.1G\lib"\
 -llibopencv_world411

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
