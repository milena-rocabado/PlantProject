#ifndef THRESHOLDING_H
#define THRESHOLDING_H

#include <opencv2/opencv.hpp>
#include "Common.h"

class Thresholding
{
public:
    Thresholding():
        searchThreshFlag_(true),
        searchNum_(0),
        bgThresh_(-1.),
        //floorThresh_(-1.),
        dump_(false)
    {}

    void process(const cv::Mat &input, cv::Mat &output);

    void setPosition(int pos) { pos_ = pos; }

    void setInputSize(const cv::Size &size) { inputSize_ = size; }

    void setROI(const cv::Rect &roi) { roi_ = roi; }

    void setDumpDirectory(std::string dir) { wd_ = dir; }

    void setSearchThreshFlag() { searchThreshFlag_ = true; }

private:
    static constexpr int HIST_SIZE { 127 };

    static constexpr double DEFAULT_BG_THRESH { 127.0 };

    // Search parameters
    static constexpr int VALLEY_THRESHOLD { 300 };
    static constexpr int VALLEY_MIN_LENGTH { 15 };
    static constexpr int MIN_TIMES_SURPASSED { 5 };

    // Recalculate threshold this many times after breakpoint
    static constexpr int RECALCULATE_THRESH_TIMES { 5 };

    // Floor threshold search
    //static constexpr int MIN_SEQ_SIZE { 5 };
    //static constexpr double DEFAULT_FLOOR_THRESH { 20.0 };

private:
    void debug_(const cv::Mat &hist, char c);

    void findThreshold_(const cv::Mat &frame);

    void findBackgroundThresh_(const cv::Mat &hist);

    // void findFloorThresh_(const cv::Mat &hist); ---> testing

    double otsuThreshold_(const cv::Mat &input, cv::Mat &output);

private:
    // Config params
    cv::Size inputSize_;
    cv::Rect roi_;

    // First processing
    bool searchThreshFlag_;
    // Times threshold recalculated
    int searchNum_;

    // Current position
    int pos_;

    // Calculated threshold for background
    double bgThresh_;
    // Calculated threshold for floor
    // double floorThresh_; --> testing

    // mean
    double brightness_;

    // aaaaa
    bool dump_;

    // Dump directory
    std::string wd_;
};

#endif
