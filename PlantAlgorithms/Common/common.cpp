#include "common.h"
#include <sstream>
#include <string>

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
void plot_size(int hist_size, cv::Size &size, cv::Size &area, int &padding) {
    int   width = hist_size * 2;
    int  height = static_cast<int>(0.75 * width);

    padding = 15;
    area = cv::Size(width, height);
    size = cv::Size(width + padding * 2, height + padding * 2);
}
// -----------------------------------------------------------------------------
void plot_size(int hist_size, cv::Size &size) {
    cv::Size area;
    int padding;
    plot_size(hist_size, size, area, padding);
}
//------------------------------------------------------------------------------
void plot_hist(const cv::Mat &hist, cv::Mat &output) {
    int padding;
    cv::Size size, area;
    plot_size(hist.size[0], size, area, padding);

    cv::Mat plot(size.height, size.width, CV_8U, cv::Scalar(255));

    double factor;
    minMaxLoc(hist, nullptr, &factor);
    factor = size.height / factor;

    for (int i = 0; i < hist.size[0]; i++) {
        float histVal = hist.at<float>(i);
        int xPos = i * 2 + padding;

        int yVal = static_cast<int>(static_cast<double>(histVal) * factor);
        cv::Point a(xPos, padding + area.height);
        cv::Point b(xPos, padding + area.height - yVal);
        cv::line(plot, a, b, cv::Scalar(0));
        a.x += 1; b.x += 1;
        cv::line(plot, a, b, cv::Scalar(0));

        if (i % 50 == 0) {
            cv::putText(plot, std::to_string(i), cv::Point(xPos, padding + area.height + 10),
                        cv::FONT_HERSHEY_SIMPLEX, 0.3, cv::Scalar(0));
        }
    }

    output = plot;
}
//------------------------------------------------------------------------------
void vertical_line(cv::Mat &img, int pos) {
    assert(pos <= img.cols);

    line(img, cv::Point(pos, 0), cv::Point(pos, img.rows), cv::Scalar(0, 0, 255));
}
//------------------------------------------------------------------------------
void plot_hist(const cv::Mat &hist, cv::Mat &output, int v_line){
    plot_hist(hist, output);
    vertical_line(output, v_line);
}
//------------------------------------------------------------------------------
template <typename T>
void plot_vector(const std::vector<T> v, cv::Mat &plot) {
    int padding = 15;
    int   width = v.size() * 2;
    int  height = static_cast<int>(0.75 * width);

    cv::Size size(width + padding * 2, height + padding * 2);
    cv::Rect mask(padding, padding, width, height);

    cv::Mat img(size.height, size.width, CV_8U, cv::Scalar(255));
    cv::Mat area = img(mask);

    T max_elem = *(max_element(v.begin(), v.end()));
    float factor = size.height / max_elem;

    for (uint i = 0; i < v.size(); i++) {
        int xPos = i * 2;
        int yVal = static_cast<int>(v[i] * factor);

        cv::Point a(xPos, height);
        cv::Point b(xPos, height - yVal);

        cv::line(plot, a, b, cv::Scalar(0), 2);
        // a.x += 1; b.x += 1;
        // cv::line(plot, a, b, cv::Scalar(0));

        // X axis labels
        if (i % 50 == 0) {
            cv::putText(img, std::to_string(i), cv::Point(xPos, padding + height + 10),
                        cv::FONT_HERSHEY_SIMPLEX, 0.3, cv::Scalar(0));
        }
    }

    // Y axis labels
    int resolution = 10;
    for (int i = 0; i < resolution; i++) {
        float value = i * max_elem / resolution;
        int pos = i * height / resolution;
        std::string text = std::to_string(static_cast<int>(value));

        cv::putText(img, text, cv::Point(padding - 10, pos),
                    cv::FONT_HERSHEY_SIMPLEX, 0.3, cv::Scalar(0));
    }

    plot = img;
}

template void plot_vector(const std::vector<float>, cv::Mat &);
