#ifndef THRESHOLDING_H
#define THRESHOLDING_H

#include <opencv2/opencv.hpp>
#include "Common.h"

class Thresholding
{
public:
    Thresholding():
        firstProcessing_(true),
        bgThresh_(-1.),
        floorThresh_(-1.),
        dump_(false)
    {}

    void process(const cv::Mat &input, common::Interval interval, cv::Mat &output);

    void setInitialPosition(int pos) { pos_ = pos; }

    void setInputSize(const cv::Size &size) { inputSize_ = size; }

    void setROI(const cv::Rect &roi) { roi_ = roi; }

    void setDumpDirectory(std::string dir) { wd_ = dir; }

private:
    static constexpr int HIST_SIZE { 127 };

    static constexpr double DEFAULT_BG_THRESH { 127.0 };
    // Background threshold search
    static constexpr int MAX_SEARCH_POS { 200 };

    // Search parameters
//    static constexpr int VALLEY_THRESHOLD { 250 };
//    static constexpr int VALLEY_MIN_LENGTH { 25 };
    static constexpr int VALLEY_THRESHOLD { 200 };
    static constexpr int VALLEY_MIN_LENGTH { 15 };

    // Floor threshold search
    static constexpr int MIN_SEQ_SIZE { 5 };
    static constexpr double DEFAULT_FLOOR_THRESH { 20.0 };

private:
    void debug_(const cv::Mat &hist, char c);

    void findThreshold_(const cv::Mat &frame, common::Interval interval);

    void findBackgroundThresh_(const cv::Mat &hist, common::Interval interval);

    void findFloorThresh_(const cv::Mat &hist); // ---> testing

    double otsuThreshold_(const cv::Mat &input, cv::Mat &output);

private:
    // Config params
    cv::Size inputSize_;
    cv::Rect roi_;

    // First processing
    bool firstProcessing_;

    // Current position
    int pos_;

    // Calculated threshold for background
    double bgThresh_;
    // Calculated threshold for floor
    double floorThresh_; // --> testing
    // threshold calculated on last call
    double lastThresh_;

    // aaaaa
    bool dump_;

    // Dump directory
    std::string wd_;
};

#endif
