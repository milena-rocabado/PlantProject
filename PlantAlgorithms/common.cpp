#include "common.h"

void crop_roi(const cv::Mat &src, cv::Mat &dst) {
    cv::Rect roi(src.cols/3, 0, src.cols/3, src.rows - BAR_HEIGHT);
    dst = src(roi);
}

void save(const std::string &fn, const cv::Mat &img) {
    imwrite(WD + fn + ".png", img);
}
