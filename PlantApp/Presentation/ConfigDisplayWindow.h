#ifndef CONFIGDISPLAYWINDOW_H
#define CONFIGDISPLAYWINDOW_H

#include <opencv2/opencv.hpp>

class ConfigDisplayWindow
{
public:
    ConfigDisplayWindow();

    void setImage(const cv::Mat &image);

    void display(const cv::Rect &roi, int yPot);

private:
    static inline const std::string WIN_NAME = "Current configuration";
    static const cv::Scalar ROI_COLOR;
    static const cv::Scalar POT_COLOR;
    static constexpr int THICKNESS { 3 };

private:

    // Image that will be displayed
    cv::Mat image_;
};

#endif // CONFIGDISPLAYWINDOW_H
