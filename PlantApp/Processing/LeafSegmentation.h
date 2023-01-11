#ifndef LEAFSEGMENTATION_H
#define LEAFSEGMENTATION_H

#include <opencv2/opencv.hpp>

class LeafSegmentation
{
public:
    LeafSegmentation();

    void initialize();

    void process(const cv::Mat &input,cv::Mat &l_output,  cv::Mat &r_output, cv::Mat &stem);

    void setPosition(int pos) { pos_ = pos; }

    void setROI(const cv::Rect &roi) { roi_ = roi; }

    int getPotPosition() { return potPosition_; }

    void setPotPosition(int pos) { potPosition_ = pos; }

    void resetPotPosition() { potPosition_ = DEFAULT_POT_POSITION; }

    void setDumpDirectory(std::string dir) { wd_ = dir; }

private:
    // Maximum width of stem
    static constexpr int MAX_WIDTH { 10 };
    // Maximum difference in position of bottom of stem in subsequent calls
    static constexpr int MAX_DIFF { 6 };
    // Maximum search radius, used in lower half to avoid leaves
    static constexpr int MAX_RADIUS { 6 };

    // Y position of top corner of plant pot
    static constexpr int DEFAULT_POT_POSITION { 567 };
    static constexpr int TIMEBAR_HEIGHT { 20 };

    // Structural element size for clean-up morphological operation
    static constexpr int ELEMENT_SIZE { 5 }; //3

    // Background color
    inline static const uchar BG_COLOR { 0 };

private:
    bool isStem_(const cv::Vec3b &px);

    void searchSides_(cv::Mat &ref, cv::Mat &lRef, cv::Mat &rRef,
                      int row, int last,
                      bool left, bool right);

    void search_(cv::Mat &ref, cv::Mat &lRef, cv::Mat &rRef);

    void colorOverLeaves_(const cv::Point &point, cv::Mat &left, cv::Mat &right);

    void hidePlantPot_(cv::Mat &image);

    void colorFromVector_(cv::Mat &ref, cv::Mat &lRef, cv::Mat &rRef);

    void cleanUp_(cv::Mat &image);

private:
    // Config params
    cv::Rect roi_; // already without timebar

    // Current position in video input
    int pos_;

    // Vertical position of plant pot
    int potPosition_;
    // Horizontal position of end of stem
    int stemPosition_;
    // Maximum row to search
    int maxRow_;
    // If we have searched up to maximum width before, limit search radius
    bool maxWidthReached_;

    // Stores stem position for each row
    std::vector<int> stem_;
    std::vector<int> lastStem_;

    // Process function called for the first time
    bool firstProcessing_;

    // Output dump directory
    std::string wd_;

};

#endif
