#include <string>
#include <opencv2/opencv.hpp>
#include <Utils.h>

std::string common::outfilename(const std::string &filename, const std::string &suffix) {
    size_t lastdot = filename.find_last_of(".");
    if (lastdot == std::string::npos) return filename + suffix;
    return filename.substr(0, lastdot) + suffix;
}
// ------------------------------------------------------------------
std::string common::get_wd_from(const std::string &path) {
    size_t last_slash = path.find_last_of("/");
    if (last_slash != std::string::npos) {
        return path.substr(0, last_slash + 1);
    } else
        return "";
}
// ------------------------------------------------------------------
std::string common::build_abs_path(const std::string &wd, const std::string &path) {
    assert(!wd.empty());
    std::stringstream ss;
    ss << wd << path;
    return ss.str();
}
// ------------------------------------------------------------------
void common::save_image(const std::string &wd, const cv::Mat &im, const std::string &dir, const std::string &filename) {
    std::string fp = common::build_abs_path(wd, "Ejemplos/"  + dir + "/" + filename + ".png");
    imwrite(fp, im);
}
// ------------------------------------------------------------------
void common::save_image(const std::string &wd, const cv::Mat &im, const std::string &filename) {
    std::string fp = common::build_abs_path(wd, "Ejemplos/" + filename + ".png");
    imwrite(fp, im);
}
// ------------------------------------------------------------------
void common::show_image(const cv::Mat &frame, const std::string &name) {
    cv::namedWindow(name, cv::WINDOW_NORMAL);
    cv::moveWindow(name, 0, 0);
    cv::resizeWindow(name, frame.size().width, frame.size().height);
    imshow(name, frame);
    cv::waitKey(1);
}
// ------------------------------------------------------------------
// ------------------------------------------------------------------
void common::apply_mask(const cv::Mat &img, const cv::Mat &mask, cv::Mat &dst) {
    dst.release();
    img.copyTo(dst, mask);
}
// ------------------------------------------------------------------
void common::crop_time_bar(const cv::Mat &src, cv::Mat &dst) {
    cv::Rect roi(0, 0, src.size().width, src.size().height - BAR_HEIGHT);
    dst = src(roi);
}
// ------------------------------------------------------------------
void common::crop_time_bar(cv::Mat &img) {
    crop_time_bar(img, img);
}
// ------------------------------------------------------------------
void common::crop_roi(const cv::Mat &src, cv::Mat &dst) {
    cv::Rect roi(src.cols/3, 0, src.cols/3, src.rows - BAR_HEIGHT);
    dst = src(roi);
}
// ------------------------------------------------------------------
void common::crop_roi(cv::Mat &img) {
    crop_roi(img, img);
}
// ------------------------------------------------------------------
// ------------------------------------------------------------------
void common::plot_hist(const cv::Mat &hist, cv::Mat &output) {
    int padding;
    cv::Size size, area;
    common::plot_size(hist.size[0], size, area, padding);

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
// ------------------------------------------------------------------
void common::plot_size(int hist_size, cv::Size &size, cv::Size &area, int &padding) {
    int   width = hist_size * 2;
    int  height = static_cast<int>(0.75 * width);

    padding = 15;
    area = cv::Size(width, height);
    size = cv::Size(width + padding * 2, height + padding * 2);
}
// ------------------------------------------------------------------
void common::plot_size(int hist_size, cv::Size &size) {
    cv::Size area;
    int padding;
    plot_size(hist_size, size, area, padding);
}
// ------------------------------------------------------------------
