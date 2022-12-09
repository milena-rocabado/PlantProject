#ifndef ROIWINDOW_H
#define ROIWINDOW_H

#include <opencv2/opencv.hpp>

class AnalyzerManager;

class ROIWindow
{
public:
    ROIWindow();

    void setImage(const cv::Mat &image);

    cv::Rect getROI();

private:
    static inline const std::string WIN_NAME = "Select ROI";

private:

    // Image that will be displayed
    cv::Mat image_;

};

#endif // ROIWINDOW_H
