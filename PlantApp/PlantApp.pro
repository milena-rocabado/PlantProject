#-------------------------------------------------
#
# Project created by QtCreator 2022-11-14T18:24:03
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = PlantApp
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++17

SOURCES += \
    Common/Common.cpp \
    Common/Plots.cpp \
    Common/Utils.cpp \
    Presentation/ConfigDisplayWindow.cpp \
    Presentation/PotPositionWindow.cpp \
        Processing/AnalyzerManager.cpp \
        Processing/PreProcessing.cpp \
        Processing/DayOrNight.cpp \
        Processing/Thresholding.cpp \
        Processing/LeafSegmentation.cpp \
        Processing/EllipseFitting.cpp \
        Presentation/MainWindow.cpp \
        Presentation/ROIWindow.cpp \
        main.cpp \

HEADERS += \
        Common/Colors.h \
        Common/Common.h \
        Common/Histograms.h \
        Common/Plots.h \
        Common/Traces.h \
        Common/Utils.h \
        Presentation/ConfigDisplayWindow.h \
        Presentation/PotPositionWindow.h \
        Processing/AnalyzerManager.h \
        Processing/PreProcessing.h \
        Processing/DayOrNight.h \
        Processing/Thresholding.h \
        Processing/LeafSegmentation.h \
        Processing/EllipseFitting.h \
        Presentation/MainWindow.h \
        Presentation/ROIWindow.h

FORMS += \
      Presentation/MainWindow.ui

INCLUDEPATH += \
            "C:\OpenCV\OpenCV4.1.1G\include" \
            "Common/" \
            "Processing/" \
            "Presentation/"

LIBS += \
     -L"C:\OpenCV\OpenCV4.1.1G\lib" \
     -llibopencv_world411

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DEFINES += TRACE_ON \
           DUMP_ON \
#           OTSU
