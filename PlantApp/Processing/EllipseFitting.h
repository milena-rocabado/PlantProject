#ifndef ELLIPSEFITTING_H
#define ELLIPSEFITTING_H

#include <opencv2/opencv.hpp>

#include "Enums.h"

class EllipseFitting
{
public:
    EllipseFitting() {}

    void process(const cv::Mat &leaf, const cv::Mat &canvas, cv::Mat &ellipseDrawing,
                 float &angle, common::Side side);

    void setInitialPosition(int pos) { pos_ = pos; }

    void setROI(const cv::Rect &roi) { roi_ = roi; }

private:
    inline static const cv::Vec3b RED { 0, 0, 255 };
    inline static const cv::Vec3b GREEN { 0, 255, 0 };

private:
    void close_(int size, cv::Mat &image);
    void fitEllipse_(const cv::Mat &input, cv::Mat &output,
                     float &angle);
    void drawEllipse_(cv::Mat &canvas, const cv::RotatedRect &ellipse);

private:
    // Config params
    cv::Rect roi_;

    // Current position
    int pos_;

    // Contour drawing
    cv::Mat contoursOut_;
};

#endif
