#ifndef ELLIPSEFITTING_H
#define ELLIPSEFITTING_H

#include <opencv2/opencv.hpp>

#include "Common.h"

class EllipseFitting
{
public:
    EllipseFitting() {}

    void process(const cv::Mat &leaf, const cv::Mat &canvas, cv::Mat &ellipseDrawing,
                 float &angle, common::Side side);

    void setInitialPosition(int pos) { pos_ = pos; }

    void setROI(const cv::Rect &roi) { roi_ = roi; }

    void setDumpDirectory(std::string dir) { wd_ = dir; }

    cv::Mat getContoursLOut() { return contoursLOut_; }

    cv::Mat getContoursROut() { return contoursROut_; }

private:
    void findLeafContour_(const std::vector<std::vector<cv::Point>> &contours,
                          std::vector<cv::Point> &leafContour,
                          cv::Mat &contoursOut);
    void fitEllipse_(const cv::Mat &input, cv::Mat &output,
                     cv::Mat &contoursOut, float &angle);
    void drawEllipse_(cv::Mat &canvas, const cv::RotatedRect &ellipse);

private:
    // Config params
    cv::Rect roi_;

    // Current position
    int pos_;
    // Current side
    common::Side side_;

    // Contour drawing
    cv::Mat contoursLOut_;
    cv::Mat contoursROut_;

    // Output directory
    std::string wd_;
};

#endif
