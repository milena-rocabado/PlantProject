#ifndef PREPROCESSINGSTAGE_H
#define PREPROCESSINGSTAGE_H

#include <opencv2/opencv.hpp>
#include "Histograms.h"

class PreProcessing
{
public:
    PreProcessing() {}

    void initialize();

    void process(const cv::Mat &input, cv::Mat &output);

    void setInitialPosition(int pos) { pos_ = pos; }

    void setInputSize(const cv::Size &size){ inputSize_ = size; }

    void setROI(const cv::Rect &roi){ roi_ = roi; }

private:
    static constexpr float PERCENTILE = 0.015f;

private:
    void toGrayscale_(const cv::Mat &input, cv::Mat &output);

    void stretchHistogram_(const cv::Mat &input, cv::Mat &output);

private:
    // Config params
    cv::Size inputSize_;
    cv::Rect roi_;

    // Current position
    int pos_;

    // For histogram calculation
    hist::Histogram_t calcHist_;

    // Find min and max values in histogram
    float numPx_;
    // Output container for toGrayscale_ operation
    cv::Mat grayscaleOut_;
    // Output container for stretchHistogram_ operation
    cv::Mat stretchOut_;
    // Output histogram
    cv::Mat outHist_;
};

#endif
