#include "Histograms.h"
#include "Traces.h"

namespace hist {

static const int PADDING { 15 };

static const int PLOT_FONT_FACE { cv::FONT_HERSHEY_SIMPLEX };
static const int PLOT_FONT_THICKNESS { 1 };
static const double PLOT_FONT_SCALE { 0.3 };

static const cv::Scalar PLOT_AXIS_COLOR { 170, 170, 170 };
static const cv::Scalar PLOT_LINE_COLOR {   0,   0, 255 };

void plotHist(const cv::Mat &hist, cv::Mat &output, int n) {
    assert(n == -1 || n <= hist.size[0]);

    int width = hist.size[0];
    int height = static_cast<int>(0.75 * width);
    cv::Size area_size(width, height);
    cv::Size plot_size(width + 2*PADDING, height + 2*PADDING);

    cv::Mat plot(plot_size.height, plot_size.width, CV_8UC3, cv::Scalar(255, 255, 255));
    cv::Rect area_rect(PADDING, PADDING, area_size.width, area_size.height);
    cv::Mat area = plot(area_rect);

    cv::rectangle(plot, area_rect, PLOT_AXIS_COLOR);

    double factor { 0.038616 };
    // { 0.010284 }; //{ 0.011717 };
//    minMaxLoc(hist, nullptr, &factor);
//    factor = area_size.height / factor;
//    TRACE("plotHist(): ********** factor = %f\n", factor);

    for (int i = 0; i < area_size.width; i++) {
        // X axis labels
        if (i % 50 == 0) {
            cv::line(plot, { PADDING + i, PADDING + area_size.height },
                     { PADDING + i, PADDING + area_size.height + 2},
                     PLOT_AXIS_COLOR, 1);

            cv::Size text_sz = cv::getTextSize(std::to_string(i), PLOT_FONT_FACE,
                                               PLOT_FONT_SCALE, PLOT_FONT_THICKNESS, nullptr);
            cv::Point text_pos(PADDING + i - text_sz.width/2, PADDING + area_size.height + 10);
            cv::putText(plot, std::to_string(i), text_pos,
                        PLOT_FONT_FACE, PLOT_FONT_SCALE, cv::Scalar(0));
        }

        float histVal = hist.at<float>(i);

        int val = static_cast<int>(static_cast<double>(histVal) * factor);
        if (val != 0)
            cv::line(area, { i, area_size.height }, { i, area_size.height - val }, cv::Scalar(0));

    }

    if (n != -1) {
        cv::line(area, cv::Point(n, 0), cv::Point(n, area_size.height),
                 cv::Scalar(0, 0, 255), 1);
    }

    output = plot;
}

}

