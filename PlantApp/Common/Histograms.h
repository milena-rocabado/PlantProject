#ifndef HISTOGRAMS_H
#define HISTOGRAMS_H

#include <opencv2/opencv.hpp>

namespace hist {

static constexpr int HIST_SIZE { 256 };

typedef struct {
    const int histChannels[1] { 0 };
    int histSize[1] { HIST_SIZE };
    float histRange[2] { 0.0f, 256.0f };
    const float *histRanges[1] { histRange };
    cv::Mat histMask;

    void operator()(const cv::Mat &img, cv::Mat &hist) {
        calcHist(&img, 1, histChannels, histMask, hist, 1, histSize, histRanges);
    }
} Histogram_t;

}

#endif // HISTOGRAMS_H
