#include "PreProcessing.h"
#include "Traces.h"

//------------------------------------------------------------------------------
void PreProcessing::initialize() {
    assert(!inputSize_.empty());
    assert(!roi_.empty());

    numPx_ = inputSize_.width * inputSize_.height * PERCENTILE;
    cv::Mat mask = cv::Mat::zeros(inputSize_, CV_8UC1);
    cv::rectangle(mask, roi_, cv::Scalar(255, 255, 255), cv::FILLED);

    calcHist_.histMask = mask;
}
//------------------------------------------------------------------------------
void PreProcessing::process(const cv::Mat &input, cv::Mat &output) {
    TRACE_P(pos_, "> PreProcessing::process(%d)\n", pos_);

    // to grayscale
    toGrayscale_(input, grayscaleOut_);
    TRACE_P(pos_, "* PreProcessing::process(%d): to grayscale\n", pos_);

    // stretch hist
    stretchHistogram_(grayscaleOut_, stretchOut_);
    TRACE_P(pos_, "* PreProcessing::process(%d): histogram stretched\n", pos_);

    output = stretchOut_;

    pos_++;
    TRACE_P(pos_, "< PreProcessing::process(%d)\n", pos_);
}
//------------------------------------------------------------------------------
void PreProcessing::toGrayscale_(const cv::Mat &input, cv::Mat &output) {
    cvtColor(input, output, cv::COLOR_BGR2GRAY);
}
//------------------------------------------------------------------------------
void PreProcessing::stretchHistogram_(const cv::Mat &input, cv::Mat &output) {
    assert(!calcHist_.histMask.empty());

    // Input histogram
    cv::Mat in_hist;
    calcHist_(input, in_hist);

    double min, max;
    float num;
    int index;

    // max is top 5%
    for (index = hist::HIST_SIZE - 1, num = .0f; index > 0; index--) {
        num += in_hist.at<float>(index);
        if (num > numPx_) break;
    }
    max = static_cast<double>(index);
    // min is bottom 5%
    for (index = 0, num = .0f; index < hist::HIST_SIZE; index++) {
        num += in_hist.at<float>(index);
        if (num > numPx_) break;
    }
    min = static_cast<double>(index);

    TRACE_P(pos_, "* PreProcessing::stretchHistogram_(%d):  (in) min = %d max = %d\n",
            pos_, static_cast<int>(min), static_cast<int>(max));

    // Stretch operation
    input.convertTo(output, CV_32FC1);
    output -= min;
    output *= (255.0 / (max - min));

    output.convertTo(output, CV_8UC1);
    calcHist_(output, outHist_);
}
//------------------------------------------------------------------------------
