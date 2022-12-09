#ifndef THRESHOLDING_H
#define THRESHOLDING_H

#include <opencv2/opencv.hpp>

class Thresholding
{
public:
    Thresholding() {}

    void initialize(const cv::Mat &frame); // only for static threshold

    void process(const cv::Mat &input, cv::Mat &output);

    void setInitialPosition(int pos) { pos_ = pos; }

    void setInputSize(const cv::Size &size) { inputSize_ = size; }

    void setROI(const cv::Rect &roi) { roi_ = roi; }

private:
    static constexpr int VALLEY_THRESHOLD = 130;
    static constexpr int VALLEY_MIN_LENGTH = 25;

private:
    double findThreshold_(const cv::Mat &hist);
    double otsuThreshold_(const cv::Mat &input, cv::Mat &output);

private:
    // Config params
    cv::Size inputSize_;
    cv::Rect roi_;

    // Current position
    int pos_;

    // Calculated threshold
    double optimalThreshold_;  // only for static threshold
};

#endif
