#ifndef DAYORNIGHT_H
#define DAYORNIGHT_H

#include <opencv2/opencv.hpp>

#include "Common.h"

class DayOrNight
{
public:
    static constexpr int INTERVAL_MIN_LENGTH { 100 };

public:
    DayOrNight();

    void process(const cv::Mat &input, common::Interval &interval,
                 common::OutputDataContainer &outData);

    void setInitialPosition(int pos) { initPos_ = pos; pos_ = pos; }

private:
    static constexpr double TOLERANCE { 7. };
    static constexpr int MIN_TIMES_SURPASSED { 5 };

private:
    void dayOrNight_(const cv::Mat &input);

    void update_data_(common::OutputDataContainer &outData);

private:
    // Initial position
    int initPos_;
    // Current position
    int pos_;
    // Data output update needed
    bool update_;

    // Reference brightness value
    double lastBr_;
    // Frame position of reference brightness value
    int lastPos_;
    // Number of times difference tolerance surpassed
    int timesSurpassed_;
    // Breakpoints found
    std::vector<int> breakpoints_;

    // For brightness plot
    std::vector<double> brightnessVec_;
};

#endif
