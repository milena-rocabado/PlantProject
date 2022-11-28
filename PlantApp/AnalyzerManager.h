#ifndef ANALYZERMANAGER_H
#define ANALYZERMANAGER_H

#include <opencv2/opencv.hpp>
#include "Common.h"

class AnalyzerManager
{
public:
    AnalyzerManager();

    bool setInputPath(std::string inputPath);

    void setROI();

    void initialize();

    void run();

private:
    // Video input path
    std::string inputPath_;
    // Working directory
    std::string workingDir_;
    // Video input
    cv::VideoCapture video_;
};

#endif // ANALYZERMANAGER_H
