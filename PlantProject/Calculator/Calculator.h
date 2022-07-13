#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <opencv2/opencv.hpp>

class Calculator
{
public:
    static constexpr int OUTPUT_HEIGHT_ADJ = 360;

    Calculator() : _first(true) {}

    void process_frame(const cv::Mat &frame, cv::Mat &dst);

private:
    cv::VideoWriter _contours;
    bool _first;

    void crop_frame(const cv::Mat &frame, cv::Mat &dst);
    void find_dot(cv::Mat &frame);

};

#endif // CALCULATOR_H
