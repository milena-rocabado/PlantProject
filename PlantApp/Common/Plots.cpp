#include "Plots.h"

#include "Histograms.h"

namespace utils {

// Constants -------------------------------------------------------------------
static const int PADDING { 15 };

static const int PLOT_FONT_FACE { cv::FONT_HERSHEY_SIMPLEX };
static const int PLOT_FONT_THICKNESS { 1 };
static const double PLOT_FONT_SCALE { 0.3 };

static const cv::Scalar PLOT_AXIS_COLOR { 170, 170, 170 };
static const cv::Scalar PLOT_LINE_COLOR {   0,   0, 255 };
//------------------------------------------------------------------------------
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

    double factor;
    if (hist.size[0] < hist::HIST_SIZE)
        factor = 0.005586;
    else
        factor = 0.038616*2;
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
//------------------------------------------------------------------------------
void plotVector(std::vector<double> vector, cv::Mat &plot, int accum) {

    int width = static_cast<int>(ceil(vector.size() / accum));
    int height = static_cast<int>(0.75 * width);
    cv::Size area_size(width, height);
    cv::Size plot_size(width + 2*PADDING, height + 2*PADDING);

    plot.create(plot_size, CV_8UC3);
    cv::Rect area_rect(cv::Point(PADDING, PADDING), area_size);
    cv::Mat area = plot(area_rect);

    double factor = area_size.height / 255.;

    int sum { 0 }, num { 0 }, pos { 0 };
    for (uint i = 0; i < vector.size(); i++) {
        sum += vector[i];
        num++;

        if (num % accum == 0) {
            double val = static_cast<double>(sum) / num;
            sum = 0; num = 0;

            // X axis labels
            if (pos % 50 == 0) {
                cv::line(plot,
                        { PADDING + static_cast<int>(pos), PADDING + area_size.height },
                        { PADDING + static_cast<int>(pos), PADDING + area_size.height + 2},
                        PLOT_AXIS_COLOR);

                cv::Size text_sz = cv::getTextSize(std::to_string(pos), PLOT_FONT_FACE,
                                                   PLOT_FONT_SCALE, PLOT_FONT_THICKNESS, nullptr);
                cv::Point text_pos(PADDING + pos - text_sz.width/2, PADDING + area_size.height + 10);
                cv::putText(plot, std::to_string(pos), text_pos,
                            PLOT_FONT_FACE, PLOT_FONT_SCALE, cv::Scalar(0));
            }

            int y = static_cast<int>(static_cast<double>(val) * factor);
            if (y != 0)
                cv::line(area, { pos, area_size.height }, { pos, area_size.height - y }, cv::Scalar(0));

            pos++;
        }
    }
}

} // namespace utils
