#ifndef ANALYZER_H
#define ANALYZER_H

#include <opencv2/opencv.hpp>
#include <Segmentator.h>
#include <Calculator.h>

class Analyzer
{
public:

    enum SegmType {
        Global,
        StaticModel
    };

    Analyzer(const SegmType &segmType = Global);

    bool set_video(const std::string &);
    void set_segmentator();
    void process_video();

private:
    cv::VideoCapture _video;
    cv::VideoWriter _mask;
    cv::VideoWriter _sth;
    std::string _wd;
    int _pos;

    Segmentator *_segm;
    Calculator *_calc;

    bool set_video(const std::string &path, const bool &isColor, const bool &isCropped);
    bool open_video_writer(cv::VideoWriter &, const std::string &file, const bool &isColor, const int &height_adj);
    std::string outfilename(const std::string &filename, const std::string &suffix);
    bool get_wd_from(const std::string &path);
};

#endif // ANALYZER_H
