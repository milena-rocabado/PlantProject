#ifndef COMMON_H
#define COMMON_H

#include <opencv2/opencv.hpp>

#define HIST_SIZE 256

#define BAR_HEIGHT 20

#define NIGHT_SAMPLE 30
#define DAY_SAMPLE 1050

#define WD "C:/Users/milena/git/PlantProject/Media/Ejemplos/"

void crop_roi(const cv::Mat &src, cv::Mat &dst);
void save(const std::string &fn, const cv::Mat &img);

typedef struct {
    const int histChannels[1] { 0 };
    int histSize[1] { HIST_SIZE };
    float histRange[2] { 0.0f, 256.0f };
    const float *histRanges[1] { histRange };
    cv::Mat _mask;

    void set_hist_size(int size) { histSize[0] = size; }

    void set_mask(const cv::Mat &mask) { _mask = mask; }

    void operator()(const cv::Mat &img, cv::Mat &hist) {
        calcHist(&img, 1, histChannels, _mask, hist, 1, histSize, histRanges);
    }

} Histogram;

#endif // COMMON_H
