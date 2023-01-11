#include "Thresholding.h"

#include "Traces.h"
#include "Histograms.h"
#include "Plots.h"

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
    utils::plotHist(hist, plot, static_cast<int>(bgThresh_/2));
    DUMP(plot, wd_, "debug_%d_hist_case_%c.png", pos_, c);

    dump_ = true;
}
//------------------------------------------------------------------------------
void Thresholding::findBackgroundThresh_(const cv::Mat &hist) {
    TRACE("> Thresholding::findBackgroundThresh_(%d)", pos_);

    auto equal = [](float a, float b) -> bool {
        return abs(static_cast<double>(a)-static_cast<double>(b)) < 0.001;
    };

    int valleyLen { 0 }, initBin { -1 }, endBin { -1 };
    int timesSurpassed { 0 }; // number of growing values
    bool valleyFound { false };

    for (int i = 0; i < hist.size[0]; i++) {
        float val = hist.at<float>(i);

        if (val >= VALLEY_THRESHOLD || (equal(0, val) && timesSurpassed != 0)) {
            timesSurpassed++;

            TRACE_IF(valleyLen > 0, "* Thresholding::findBackgroundThresh_(%d): "
                                    "[%d, %.0f] -> VALLEY_THRESHOLD surpassed %d times",
                     pos_, i, static_cast<double>(val), timesSurpassed);

            if (timesSurpassed > MIN_TIMES_SURPASSED) {
                // Valley too small -> restart valley
                if (valleyLen < MIN_TIMES_SURPASSED) { // at least as big as growing sequence
                    TRACE_IF(valleyLen > 0, "* Thresholding::findBackgroundThresh_(%d): "
                                            "[%d, %.0f] -> valley too small (%d), reset valley",
                             pos_, i, static_cast<double>(val), valleyLen);
                    valleyLen = 0;
                }
                // Valley big enough -> end loop
                else {
                    TRACE("* Thresholding::findBackgroundThresh_(%d): valley found !!", pos_);
                    valleyFound = true;
                    break;
                }
            }
        } else { // Value under VALLEY_THRESHOLD
            // If start of streak, store initial bin
            if (valleyLen == 0) {
                TRACE("* Thresholding::findBackgroundThresh_(%d): "
                      "[%d, %.0f] -> valley start",
                      pos_, i, static_cast<double>(val));
                initBin = i;
            }
            endBin = i;

            // Update valley length
            valleyLen++;
            timesSurpassed = 0;

            TRACE("* Thresholding::findBackgroundThresh_(%d): "
                  "[%d, %.0f] -> valley end updated, valleyLen = %d",
                  pos_, i, static_cast<double>(val), valleyLen);
        }
    }

    if (valleyFound) {
        bgThresh_ = endBin * 2;

        TRACE("* Thresholding::findBackgroundThresh_(%d): threshold found = %.0f",
              pos_, bgThresh_);

        //DELETE___
        if (bgThresh_ < 100) {
            TRACE_ERR("* Thresholding::searchHistogram_(%d): error case b: "
                      "initBin = %d endBin= %d thresh = %.0f", pos_, initBin, endBin, bgThresh_);
            debug_(hist, 'b');
        } else if (bgThresh_ > 130) {
            TRACE_ERR("* Thresholding::searchHistogram_(%d): error case c: "
                      "initBin = %d endBin= %d thresh = %.0f", pos_, initBin, endBin, bgThresh_);
            debug_(hist, 'c');
        } /*else if (pos_ >= 432 && pos_ <= 437) {
            TRACE_ERR("* Thresholding::searchHistogram_(%d): breakpoint dump "
                      "bgThresh = %.0f", pos_, bgThresh_);
            debug_(hist, 'x');
        }*/ //___
    } else {
        TRACE_ERR("* Thresholding::searchHistogram_(%d): background threshold not found, using %.0f",
                  pos_, DEFAULT_BG_THRESH);
        debug_(hist, 'a');
        searchNum_--; // Don't count as valid search

        bgThresh_ = DEFAULT_BG_THRESH;
    }

    TRACE("< Thresholding::findBackgroundThresh_(%d)", pos_);
}
//------------------------------------------------------------------------------
/*
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
*/
//------------------------------------------------------------------------------
void Thresholding::findThreshold_(const cv::Mat &frame) {

    // Calculate histogram
    hist::Histogram_t calculateHist;
    cv::Mat hist;
    calculateHist.histSize[0] = HIST_SIZE;
    calculateHist(frame(roi_), hist);

    // Search histogram for thresholds
    findBackgroundThresh_(hist);
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

    if (searchThreshFlag_) {
        findThreshold_(input);
        searchNum_++;

        if (searchNum_ == RECALCULATE_THRESH_TIMES) {
            searchThreshFlag_ = false;
            searchNum_ = 0;
        }
    }

    cv::threshold(input, output, bgThresh_, 255.0, cv::THRESH_BINARY);

//    double br = cv::mean(input(roi_))[0];
//    std::cout << "*****[" << pos_ << "] br now = " << br << " --- prev br = " << brightness_
//              << " --- diff = " << abs(br-brightness_)
//              << " --- threshold = " << bgThresh_ << std::endl;
//    brightness_ = br;

#endif

    output = 255 - output;

    DUMP_IF(dump_, output, wd_, "debug_%d_thresh_output.png", pos_);
    dump_ = false;

    TRACE_P(pos_, "* Thresholding::process(%d): bgthresh = %.0f",
            pos_, bgThresh_);
    TRACE_P(pos_, "< Thresholding::process(%d)", pos_);
    pos_++;
}
//------------------------------------------------------------------------------
