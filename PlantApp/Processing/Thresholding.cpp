#include "Thresholding.h"

#include "Traces.h"
#include "Histograms.h"

#include <cmath>

//------------------------------------------------------------------------------
void Thresholding::debug_(const cv::Mat &hist, char c) {
    // Print hist values
    TRACE_ERR("* Thresholding::debug_(%d, %c): histogram values:", pos_, c);
    for (int i = 0; i < HIST_SIZE; i++) {
        if (i % 10 == 0)
            std::cout << std::endl << "\t[" << i << "] ";
        std::cout << hist.at<float>(i) << " ";
    }
    std::cout << std::endl;

    cv::Mat plot;
    hist::plotHist(hist, plot, static_cast<int>(bgThresh_/2));
    DUMP(plot, wd_, "debug_%d_hist_case_%c.png", pos_, c);

    dump_ = true;
}
//------------------------------------------------------------------------------
void Thresholding::findBackgroundThresh_(const cv::Mat &hist, common::Interval interval) {

    int valleyLen { 0 }, initBin { -1 }, endBin { -1 };
    bool valleyFound { false };

//    for (int i = 0; i < MAX_SEARCH_POS; i++) {
    for (int i = 0; i < hist.size[0]; i++) {
        float val = hist.at<float>(i);

        if (val < VALLEY_THRESHOLD) {
            // If start of streak, store initial bin
            if (valleyLen == 0)
                initBin = i;
            endBin = i;

            // Update valley length
            valleyLen++;

        } else {
            if (valleyFound)
                // If valley found, end loop
                break;
            else
                // else, restart valley
                valleyLen = 0;
        }
    }

    if (valleyFound) {
//        bgThresh_ = endBin - (interval == common::NIGHT ? 0 : VALLEY_MIN_LENGTH / 2);
        bgThresh_ = endBin * 2;

        //DELETE___
        if (bgThresh_ < 100) {
            TRACE_ERR("* Thresholding::searchHistogram_(%d): error case b: "
                      "initBin = %d endBin= %d", pos_, initBin, endBin);
            debug_(hist, 'b');
        } else if ((lastThresh_ - bgThresh_) > 10) {
            TRACE_ERR("* Thresholding::searchHistogram_(%d): error case c: "
                      "bgThresh = %.0f lastThresh = %.0f", pos_, bgThresh_, lastThresh_);
            debug_(hist, 'c');
        } else if (pos_ >= 432 && pos_ <= 437) {
            TRACE_ERR("* Thresholding::searchHistogram_(%d): breakpoint dump "
                      "bgThresh = %.0f", pos_, bgThresh_);
            debug_(hist, 'x');
        }//___
    } else {
        TRACE_ERR("* Thresholding::searchHistogram_(%d): background threshold not found, using %.0f",
                  pos_, lastThresh_);
        debug_(hist, 'a');

        bgThresh_ = lastThresh_;
    }
}
//------------------------------------------------------------------------------
void Thresholding::findFloorThresh_(const cv::Mat &hist) {
    auto equals = [] (float a, float b) -> bool {
        return abs(a-b) < 0.001f;
    };

    float lastVal = 256.f;
    int times = 0;
    int minimum = 0;
    bool found = false;

    for (int i = 0; i < hist.size[0]; i++) {
        float val = hist.at<float>(i);

        // Ignore zeroes in histogram
        if (equals(val, 0.f)) continue;

        // If value smaller than last value, inside descending sequence
        if (val < lastVal)
            times++;
        else if (found) // descending sequence ended
            break;
        else
            times = 0;

        // If current sequence is big enough
        if (times >= MIN_SEQ_SIZE) {
            // Descending sequence found
            found = true;
            // Update minimum
            minimum = i;
        }

        lastVal = val;
    }

    floorThresh_ = minimum - 10;
}
//------------------------------------------------------------------------------
void Thresholding::findThreshold_(const cv::Mat &frame, common::Interval interval) {

    // Calculate histogram
    hist::Histogram_t calculateHist;
    cv::Mat hist;
    calculateHist.histSize[0] = HIST_SIZE;
    calculateHist(frame(roi_), hist);

    // Search histogram for thresholds
    findBackgroundThresh_(hist, interval);
//    findFloorThresh_(hist);
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
//    cv::inRange(input, floorThresh_, bgThresh_, output);

#endif

    output = 255 - output;
    lastThresh_ = bgThresh_;

    if (dump_) {
        DUMP(output, wd_, "debug_%d_thresh_output.png", pos_);
        dump_ = false;
    }

    TRACE_P(pos_, "* Thresholding::process(%d): floorthresh = %.0f bgthresh = %.0f",
            pos_, floorThresh_, bgThresh_);
    TRACE_P(pos_, "< Thresholding::process(%d)", pos_);
    pos_++;
}
//------------------------------------------------------------------------------
