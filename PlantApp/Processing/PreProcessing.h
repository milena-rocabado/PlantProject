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

    void setPosition(int pos) { pos_ = pos; }

    void setInputSize(const cv::Size &size){ inputSize_ = size; }

    void setROI(const cv::Rect &roi){ roi_ = roi; }

    void setDumpDirectory(std::string dir) { wd_ = dir; }

private:
    // For histogram stretching
    static constexpr float PERCENTILE = 0.015f;
    // Gaussian blur kernel size
    static constexpr int K_SIZE = 7;

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

    // Output dump directory
    std::string wd_;
};

#endif
