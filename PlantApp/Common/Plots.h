#ifndef PLOTS_H
#define PLOTS_H

#include <opencv2/core/mat.hpp>

namespace utils {

static const int VEC_PLOT_ACCUM { 5 };

void plotHist(const cv::Mat &hist, cv::Mat &output, int n = -1);

void plotVector(std::vector<double> vector, cv::Mat &output, int accum = VEC_PLOT_ACCUM);

}

#ifdef DUMP_ON

#include "Histograms.h"

#define DUMP_HIST(mat__, wd__, file__, ...) { \
    hist::Histogram_t calc__; \
    cv::Mat hist__, plot__; \
    calc__(mat__, hist__); \
    utils::plotHist(hist__, plot__); \
    \
    char buf__[256]; \
    std::sprintf(buf__, file__, __VA_ARGS__); \
    cv::imwrite(std::string(wd__) + std::string(buf__), plot__); \
}

#define DUMP_HIST_P(pos__, mat__, wd__, file__, ...) { \
    if (/*pos__ % N_TRACE == 0 ||*/ pos__ == 150 || pos__ == 750) { \
        DUMP_HIST(mat__, wd__, file__, __VA_ARGS__); \
    } \
}

#define DUMP_PLOT(vec__, mat__, wd__, file__, ...) { \
    cv::Mat plot__; \
    utils::plotVector(vec__, plot__); \
    \
    char buf__[256]; \
    std::sprintf(buf__, file__, __VA_ARGS__); \
    cv::imwrite(std::string(wd__) + std::string(buf__), plot__); \
}

#define DUMP_PLOT_P(pos__, vec__, mat__, wd__, file__, ...) { \
    if (pos__ % N_TRACE == 0) { \
        DUMP_PLOT(vec__, mat__, wd__, file__, __VA_ARGS__); \
    } \
}

#else

#define DUMP_HIST(mat__, wd__, file__, ...) {}
#define DUMP_HIST_P(pos__, mat__, wd__, file__, ...) {}

#endif

#endif // PLOTS_H
