#include "common.h"
#include <sstream>
#include <string>

static const int PLOT_FONT_FACE { cv::FONT_HERSHEY_SIMPLEX };
static const int PLOT_FONT_THICKNESS { 1 };
static const double PLOT_FONT_SCALE { 0.3 };
static const cv::Scalar PLOT_AXIS_COLOR { 170, 170, 170 };

void crop_roi(const cv::Mat &src, cv::Mat &dst) {
    cv::Rect roi(src.cols/3, 0, src.cols/3, src.rows - BAR_HEIGHT);
    dst = src(roi);
}
//------------------------------------------------------------------------------
void crop_upper_third(const cv::Mat &src, cv::Mat &dst) {
    cv::Rect roi(0, src.rows/3, src.cols, 2 * src.rows / 3);
    dst = src(roi);
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void save(const std::string &fn, const cv::Mat &img) {
    std::stringstream ss;
    ss << WD << "Ejemplos/" << fn << ".png";

    bool res = imwrite(ss.str(),  img);
}
// -----------------------------------------------------------------------------
void show(const std::string &name, const cv::Mat &img) {
    cv::namedWindow(name, cv::WINDOW_NORMAL);
    cv::moveWindow(name, 0, 0);
    cv::resizeWindow(name, img.size().width, img.size().height);
    imshow(name, img);
//    cv::waitKey(1);
}
// -----------------------------------------------------------------------------
std::string image_type(const cv::Mat &img) {
    int type = img.type();
    std::string r;

    uchar depth = type & CV_MAT_DEPTH_MASK;
    uchar chans = 1 + (type >> CV_CN_SHIFT);

    switch ( depth ) {
    case CV_8U:  r = "8U"; break;
    case CV_8S:  r = "8S"; break;
    case CV_16U: r = "16U"; break;
    case CV_16S: r = "16S"; break;
    case CV_32S: r = "32S"; break;
    case CV_32F: r = "32F"; break;
    case CV_64F: r = "64F"; break;
    default:     r = "User"; break;
    }

    r += "C";
    r += (chans+'0');

    return r;
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void get_plot_size(int hist_size, cv::Size &size, cv::Size &area, int &padding) {
    int   width = hist_size/* * 2*/;
    int  height = static_cast<int>(0.75 * width);

    padding = 15;
    area = cv::Size(width, height);
    size = cv::Size(width + padding * 2, height + padding * 2);
}
// -----------------------------------------------------------------------------
void get_plot_size(int hist_size, cv::Size &size) {
    cv::Size area;
    int padding;
    get_plot_size(hist_size, size, area, padding);
}
//------------------------------------------------------------------------------
void plot_hist(const cv::Mat &hist, cv::Mat &output, int v_line) {
    assert(v_line == -1 || v_line <= hist.size[0]);

    int padding;
    cv::Size plot_size, area_size;
    get_plot_size(hist.size[0], plot_size, area_size, padding);

    cv::Mat plot(plot_size.height, plot_size.width, CV_8UC3, cv::Scalar(255, 255, 255));
    cv::Rect area_rect(padding, padding, area_size.width, area_size.height);
    cv::Mat area = plot(area_rect);

    cv::rectangle(plot, area_rect, PLOT_AXIS_COLOR);

    double factor { 0.038616 };
//    minMaxLoc(hist, nullptr, &factor);
//    factor = area_size.height / factor;
//    TRACE(true, "plot_hist(): factor = %f\n", factor);

    for (int i = 0; i < area_size.width; i++) {
        // X axis labels
        if (i % 50 == 0) {
            cv::line(plot, { padding + i, padding + area_size.height },
                     { padding + i, padding + area_size.height + 2},
                     PLOT_AXIS_COLOR, 1);

            cv::Size text_sz = cv::getTextSize(std::to_string(i), PLOT_FONT_FACE,
                                               PLOT_FONT_SCALE, PLOT_FONT_THICKNESS, nullptr);
            cv::Point text_pos(padding + i - text_sz.width/2, padding + area_size.height + 10);
            cv::putText(plot, std::to_string(i), text_pos,
                        PLOT_FONT_FACE, PLOT_FONT_SCALE, cv::Scalar(0));
        }

        float histVal = hist.at<float>(i);

        int val = static_cast<int>(static_cast<double>(histVal) * factor);
        if (val != 0)
            cv::line(area, { i, area_size.height }, { i, area_size.height - val }, cv::Scalar(0));

    }

    if (v_line != -1)
        cv::line(area, cv::Point(v_line, 0), cv::Point(v_line, area_size.height),
                 cv::Scalar(0, 0, 255), 1);

    output = plot;
}
//------------------------------------------------------------------------------
void vertical_line(cv::Mat &img, int pos) {
    int padding = 15;
    assert(pos <= (img.cols - 2*padding));

    line(img, cv::Point(pos + padding, padding),
         cv::Point(pos + padding, img.rows - padding),
         cv::Scalar(0, 0, 255));
}
//------------------------------------------------------------------------------
