#include "Thresholding.h"

#include "Traces.h"
#include "Histograms.h"

#include <cmath>

//------------------------------------------------------------------------------
void Thresholding::searchHistogram_(const cv::Mat &hist, common::Interval interval) {

    int streak { 0 }, initBin { -1 }, endBin { -1 };
    bool valleyFound { false };

    for (int i = 0; i < MAX_SEARCH_POS; i++) {
        float val = hist.at<float>(i);

        if (val < VALLEY_THRESHOLD) {
            // If start of streak, store initial bin
            if (streak == 0)
                initBin = i;

            // Update streak
            streak++;

            // If minimum length surpassed, store/update end bin
            if (streak > VALLEY_MIN_LENGTH) {
                endBin = i;
                valleyFound = true;
            }
        } else {
            if (valleyFound) // If found, end loop
                break;
            streak = 0;
        }
    }

    if (valleyFound) {
        bgThresh_ = endBin - (interval == common::NIGHT ? 0 : VALLEY_MIN_LENGTH / 2);
    }
}
//------------------------------------------------------------------------------
void Thresholding::searchHistogram2_(const cv::Mat &hist) {
    auto equals = [] (float a, float b) -> bool {
        return abs(a-b) < 0.001f;
    };

    float lastVal = 256.f;
    int times = 0;
    int minimum = 0;
    bool found = false;
    for (int i = 0; i < hist.size[0]; i++) {
        float val = hist.at<float>(i);

        if (equals(val, 0.f)) continue;

        if (val < lastVal)
            times++;
        else if (found)
            break;
        else
            times = 0;

        lastVal = val;

        if (times >= 5) {
            found = true;
            minimum = i;
        }
    }

    floorThresh_ = minimum;
}
//------------------------------------------------------------------------------
void Thresholding::findThreshold_(const cv::Mat &frame, common::Interval interval) {

    // Calculate histogram
    hist::Histogram_t calculateHist;
    cv::Mat hist;
    calculateHist(frame(roi_), hist);

    searchHistogram_(hist, interval);
}
//------------------------------------------------------------------------------
double Thresholding::otsuThreshold_(const cv::Mat &input, cv::Mat &output) {
    cv::Mat otsu_in = input(roi_);
    cv::Mat otsu_out(otsu_in.size(), otsu_in.type()); // will be discarded

    // Get optimal threshold by applying Otsu algorithm to ROI
    double thresh = threshold(otsu_in, otsu_out, 255, 255, cv::THRESH_OTSU);

    // Apply threshold to whole image
    threshold(input, output, thresh, 255, cv::THRESH_BINARY);

    return thresh;
}
//------------------------------------------------------------------------------
void Thresholding::process(const cv::Mat &input, common::Interval interval,
                           cv::Mat &output) {
    TRACE_P(pos_, "> Thresholding::process(%d)", pos_);

#ifdef OTSU
    double threshold = otsuThreshold_(input, output);
    TRACE_P(pos_, "* Thresholding::process: threshold = %.0f", threshold);
#else
    findThreshold_(input, interval);

    cv::threshold(input, output, bgThresh_, 255.0, cv::THRESH_BINARY);
#endif

    output = 255 - output;

    TRACE_P(pos_, "< Thresholding::process(%d)", pos_);
    pos_++;
}
//------------------------------------------------------------------------------
