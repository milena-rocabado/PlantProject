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

void plotHist(const cv::Mat &hist, cv::Mat &output, int n = -1);

}

#ifdef DUMP_ON

#define DUMP_HIST(mat__, wd__, file__, ...) { \
    hist::Histogram_t calc__; \
    cv::Mat hist__, plot__; \
    calc__(mat__, hist__); \
    hist::plotHist(hist__, plot__); \
    \
    char buf__[256]; \
    std::sprintf(buf__, file__, __VA_ARGS__); \
    cv::imwrite(std::string(wd__) + std::string(buf__), plot__); \
}

#define DUMP_HIST_P(pos__, mat__, wd__, file__, ...) { \
    if (pos__ % N_TRACE == 0) { \
        DUMP_HIST(mat__, wd__, file__, __VA_ARGS__); \
    } \
}

#else

#define DUMP_HIST(mat__, wd__, file__, ...) {}
#define DUMP_HIST_P(pos__, mat__, wd__, file__, ...) {}

#endif

#endif // HISTOGRAMS_H
