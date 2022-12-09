#include "Thresholding.h"
#include "Traces.h"

#include "Utils.h"
#include "Histograms.h"

//------------------------------------------------------------------------------
void Thresholding::initialize(const cv::Mat &frame) {
    assert(!roi_.empty());
    assert(!inputSize_.empty());

    hist::Histogram_t calculateHist;
    cv::Mat mask = cv::Mat::zeros(inputSize_, CV_8UC1);
    cv::rectangle(mask, roi_, cv::Scalar(255, 255, 255), cv::FILLED);
    calculateHist.histMask = mask;

    cv::Mat hist;
    cv::cvtColor(frame, frame, cv::COLOR_BGR2GRAY);
    calculateHist(frame, hist);
    optimalThreshold_ = findThreshold_(hist);
}
//------------------------------------------------------------------------------
double Thresholding::findThreshold_(const cv::Mat &hist) {
    TRACE("> Thresholding::findThreshold_(VALLEY_THRESHOLD = %d, STREAK = %d)\n",
          VALLEY_THRESHOLD, VALLEY_MIN_LENGTH);

    int streak { 0 }, initial_bin { -1 };
    float val;

    for (int i = 0; i < hist.size[0]; i++) {
        val = hist.at<float>(i);

        if (val < VALLEY_THRESHOLD) {
            if (0 == streak++) {
                initial_bin = i;
                TRACE("* Thresholding::findThreshold_(): start streak at %d\n", i);
            }
            TRACE("* Thresholding::findThreshold_(): streak = %d\n", streak);
            if (streak == VALLEY_MIN_LENGTH) break;
        } else {
            TRACE("* Thresholding::findThreshold_(): restart streak at %d\n", i);
            streak = 0;
        }
    }

    double threshold { -1.0 };
    if (streak == VALLEY_MIN_LENGTH) {
        threshold = initial_bin + VALLEY_MIN_LENGTH/2;
        TRACE("* Thresholding::findThreshold_(): threshold found = %.2f\n", threshold);
    } else {
        TRACE("* Thresholding::findThreshold_(): threshold not found\n");
    }

    TRACE("< Thresholding::findThreshold_()\n");
    return threshold;
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
void Thresholding::process(const cv::Mat &input, cv::Mat &output) {
    TRACE_P(pos_, "> Thresholding::process(%d)", pos_);

#ifdef OTSU
    double threshold = otsuThreshold_(input, output);
    TRACE_P(pos_, "* Thresholding::process: threshold = %.0f", threshold);
#else
    cv::threshold(input, output, optimalThreshold_, 255.0, cv::THRESH_BINARY);
#endif

    TRACE_P(pos_, "< Thresholding::process(%d)", pos_);
    pos_++;
}
//------------------------------------------------------------------------------
