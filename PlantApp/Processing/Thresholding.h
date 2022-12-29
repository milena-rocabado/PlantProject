#ifndef THRESHOLDING_H
#define THRESHOLDING_H

#include <opencv2/opencv.hpp>
#include "Enums.h"

class Thresholding
{
public:
    Thresholding():
        firstProcessing_(true),
        bgThresh_(-1.)
    {}

    void process(const cv::Mat &input, common::Interval interval, cv::Mat &output);

    void setInitialPosition(int pos) { pos_ = pos; }

    void setInputSize(const cv::Size &size) { inputSize_ = size; }

    void setROI(const cv::Rect &roi) { roi_ = roi; }

private:

    static constexpr int MAX_SEARCH_POS { 200 };
//    static constexpr int VALLEY_THRESHOLD { 200 };
    static constexpr int VALLEY_THRESHOLD { 150};
    static constexpr int VALLEY_MIN_LENGTH { 20 }; // menos?

private:
    void findThreshold_(const cv::Mat &frame, common::Interval interval);

    void searchHistogram_(const cv::Mat &hist, common::Interval interval);

    void searchHistogram2_(const cv::Mat &hist); // ---> no

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
    double floorThresh_;
};

#endif
