#include "PreProcessing.h"
#include "Traces.h"
#include "Plots.h"

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
    TRACE_P(pos_, "> PreProcessing::process(%d)", pos_);
    assert(input.channels() == 1);

    DUMP_HIST_P(pos_, input(roi_), wd_, "input_hist_%d.png", pos_);

#ifdef OTSU
    // Stretch operation not needed
    output = input;

#else
    // Stretch hist
    stretchHistogram_(input, output);
    TRACE_P(pos_, "* PreProcessing::process(%d): histogram stretched", pos_);
    DUMP_HIST_P(pos_, output(roi_), wd_, "stretched_hist_%d.png", pos_);
#endif

    TRACE_P(pos_, "< PreProcessing::process(%d)", pos_);
    pos_++;
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

    // max is percentile 98.5%
    for (index = hist::HIST_SIZE - 1, num = .0f; index > 0; index--) {
        num += in_hist.at<float>(index);
        if (num > numPx_) break;
    }
    max = static_cast<double>(index);
    // min is percentile 1.5%
    for (index = 0, num = .0f; index < hist::HIST_SIZE; index++) {
        num += in_hist.at<float>(index);
        if (num > numPx_) break;
    }
    min = static_cast<double>(index);

    TRACE_P(pos_, "* PreProcessing::stretchHistogram_(%d):  (in) min = %d max = %d",
            pos_, static_cast<int>(min), static_cast<int>(max));

    // Stretch operation
    input.convertTo(output, CV_32FC1);
    output -= min;
    output *= (255.0 / (max - min));

    output.convertTo(output, CV_8UC1);

//    //DELETE________
//    auto equal = [](float a, float b) -> bool {
//        return abs(a-b) < 0.00001f;
//    };
//    int minG{-1}, maxG{-1};
//    for (int i = 0; i < in_hist.size[0]; i++) {
//        float val = in_hist.at<float>(i);
//        if (!equal(val, 0)) {
//            minG = i;
//            break;
//        }
//    }
//    for (int i = in_hist.size[0]; i > 0; i--) {
//        float val = in_hist.at<float>(i);
//        if (!equal(val, 0)) {
//            maxG = i;
//            break;
//        }
//    }
//    if (minG == -1 || maxG == -1) {
//        std::cout << "***** aaaaaaaaaaaaaaaaaaa"<< std::endl;
//        return;
//    }
//    TRACE_P(pos_, "* PreProcessing::stretchHistogram_(%d):  ***** (in) minG = %d maxG = %d",
//            pos_, minG, maxG);
//    cv::Mat outputG;
//    input.convertTo(outputG, CV_32FC1);
//    outputG -= minG;
//    outputG *= (255.0 / (maxG - minG));
//    outputG.convertTo(outputG, CV_8UC1);
//    DUMP_HIST_P(pos_, outputG(roi_), wd_, "stretched_hist_%d_global.png", pos_);
//    DUMP_P(pos_, outputG(roi_), wd_, "stretched_out_%d_global.png", pos_);
//    //________
}
//------------------------------------------------------------------------------
