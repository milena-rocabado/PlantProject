#ifndef LEAFSEGMENTATION_H
#define LEAFSEGMENTATION_H

#include <opencv2/opencv.hpp>

class LeafSegmentation
{
public:
    LeafSegmentation() {}

    void process(const cv::Mat &input, cv::Mat &r_output, cv::Mat &l_output, cv::Mat &stem);

    void setInitialPosition(int pos) { pos_ = pos; }

    void setROI(const cv::Rect &roi) { roi_ = roi; }

private:
    static constexpr int MAX_WIDTH { 6 };

#warning POT_HEIGHT should not be a constant
    static constexpr int POT_HEIGHT { 567 };
    static constexpr int TIMEBAR_HEIGHT { 20 };

    inline static const cv::Vec3b RED     {   0,   0, 255 };
    inline static const cv::Vec3b MAGENTA { 255,   0, 255 };
    inline static const cv::Vec3b CYAN    { 255, 255,   0 };
    inline static const cv::Vec3b GREEN   {   0, 255,   0 };

    inline static const uchar BG_COLOR { 255 };

private:
    void colorOverLeaves_(const cv::Point &point, cv::Mat &left, cv::Mat &right);
    void hidePlantPot_(cv::Mat &image);

private:
    // Config params
    cv::Rect roi_; // already without timebar

    // Current position
    int pos_;
};

#endif
