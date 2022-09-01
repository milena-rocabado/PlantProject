TEMPLATE = app
CONFIG += console c++20
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        GlobalSegmentator.cpp \
        PercentileSegmentator.cpp \
        common.cpp \
        main.cpp

HEADERS += \
    GlobalSegmentator.h \
    PercentileSegmentator.h \
    common.h

INCLUDEPATH += "C:\OpenCV\OpenCV4.1.1G\include"

LIBS += -L"C:\OpenCV\OpenCV4.1.1G\lib"\
 -llibopencv_world411

DEFINES += DEBUG
