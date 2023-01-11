#ifndef POTPOSITIONWINDOW_H
#define POTPOSITIONWINDOW_H

#include <opencv2/core/mat.hpp>

class PotPositionWindow
{
public:
    PotPositionWindow();

    void setImage(const cv::Mat &image);

    int getPotPosition();

private:
    static inline const std::string WIN_NAME = "Select pot position";

private:
    static void onMouse_(int event, int x, int y, int, void*);

private:
    // Base image
    cv::Mat image_;

    // Image displayed
    cv::Mat canvas_;

};

#endif // POTPOSITIONWINDOW_H
