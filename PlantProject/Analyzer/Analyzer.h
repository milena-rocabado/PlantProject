#ifndef ANALYZER_H
#define ANALYZER_H

#include <opencv2/opencv.hpp>
#include <Segmentator.h>

class Analyzer
{
public:

    enum SegmType {
        Global,
        StaticModel
    };

    Analyzer() {}

    Analyzer(const SegmType &);

    void set_segmentator();

    void process_video();

private:
    cv::VideoCapture _video;
    cv::VideoWriter _salida;

    Segmentator *_segm;
};

#endif // ANALYZER_H
