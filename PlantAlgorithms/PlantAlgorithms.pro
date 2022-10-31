TEMPLATE = app
CONFIG += console c++20
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += c++17

SOURCES += \
        Analyzer/BaseAnalyzer.cpp \
        Analyzer/BetterAnalyzer.cpp \
        Common/common.cpp \
        Segmentator/DayNightSegmentator.cpp \
        Segmentator/GlobalSegmentator.cpp \
        Segmentator/PercentileSegmentator.cpp \
        main.cpp

HEADERS += \
    Analyzer/BaseAnalyzer.h \
    Analyzer/BetterAnalyzer.h \
    Common/common.h \
    Segmentator/DayNightSegmentator.h \
    Segmentator/GlobalSegmentator.h \
    Segmentator/PercentileSegmentator.h

INCLUDEPATH += "C:\OpenCV\OpenCV4.1.1G\include" \
    Analyzer \
    Common \
    Segmentator \

LIBS += -L"C:\OpenCV\OpenCV4.1.1G\lib"\
 -llibopencv_world411

DEFINES += DEBUG \
    DESKTOP
