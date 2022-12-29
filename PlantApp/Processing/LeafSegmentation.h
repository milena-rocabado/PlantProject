#ifndef LEAFSEGMENTATION_H
#define LEAFSEGMENTATION_H

#include <opencv2/opencv.hpp>

class LeafSegmentation
{
public:
    LeafSegmentation();

    void initialize();

    void process(const cv::Mat &input,cv::Mat &l_output,  cv::Mat &r_output, cv::Mat &stem);

    void setInitialPosition(int pos) { pos_ = pos; }

    void setROI(const cv::Rect &roi) { roi_ = roi; }

    int getPotPosition() { return potPosition_; }

    void setPotPosition(int pos) { potPosition_ = pos; }

    void setDumpDirectory(std::string dir) { wd_ = dir; }

private:
    static constexpr int MAX_WIDTH { 10 };
    static constexpr int MAX_DIFF { 6 };

    static constexpr int DEFAULT_POT_POSITION { 567 };
    static constexpr int TIMEBAR_HEIGHT { 20 };

    static constexpr int ELEMENT_SIZE { 3 };

    inline static const cv::Vec3b RED     {   0,   0, 255 };
    inline static const cv::Vec3b MAGENTA { 255,   0, 255 };
    inline static const cv::Vec3b CYAN    { 255, 255,   0 };
    inline static const cv::Vec3b GREEN   {   0, 255,   0 };

    inline static const uchar BG_COLOR { 0 };

private:
    bool isStem3C(const cv::Vec3b &px);

    void search_(cv::Mat &ref, cv::Mat &lRef, cv::Mat &rRef);

    void colorOverLeaves_(const cv::Point &point, cv::Mat &left, cv::Mat &right);

    void hidePlantPot_(cv::Mat &image);

    void colorFromVector_(cv::Mat &ref, cv::Mat &lRef, cv::Mat &rRef);

    void cleanUp_(cv::Mat &image, int size);

private:
    // Config params
    cv::Rect roi_; // already without timebar

    // Current position in video input
    int pos_;

    // Vertical position of plant pot
    int potPosition_;
    // Horizontal position of end of stem
    int stemPosition_;

    // Stores stem position for each row
    std::vector<int> stem_;
    std::vector<int> lastStem_;

    // Process function called for the first time
    bool firstProcessing_;

    // Output dump directory
    std::string wd_;

};

#endif
