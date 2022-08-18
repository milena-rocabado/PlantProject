#include <string>
#include <opencv2/opencv.hpp>

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))

#define HIST_SIZE 256

#define BAR_HEIGHT 20

namespace common {

// IO

std::string outfilename(const std::string &filename, const std::string &suffix);

std::string get_wd_from(const std::string &path);

std::string build_abs_path(const std::string &wd, const std::string &path);

void save_image(const std::string &wd, const cv::Mat &im, const std::string &dir, const std::string &filename);

void save_image(const std::string &wd, const cv::Mat &im, const std::string &filename);

void show_image(const cv::Mat &frame, const std::string &name);

// MODIFY

void apply_mask(const cv::Mat &img, const cv::Mat &mask, cv::Mat &dst);

void crop_time_bar(const cv::Mat &src, cv::Mat &dst);

void crop_time_bar(cv::Mat &img);

void crop_roi(const cv::Mat &src, cv::Mat &dst);

void crop_roi(cv::Mat &img);

// HISTOGRAMS

typedef struct {
    const int histChannels[1] { 0 };
    int histSize[1] { HIST_SIZE };
    float histRange[2] { 0.0f, 256.0f };
    const float *histRanges[1] { histRange };
    cv::Mat _mask;

    void set_mask(const cv::Mat &mask) { _mask = mask; }

    void operator()(const cv::Mat &img, cv::Mat &hist) {
        calcHist(&img, 1, histChannels, _mask, hist, 1, histSize, histRanges);
    }

} Histogram;

void plot_hist(const cv::Mat &hist, cv::Mat &output);

void plot_size(int hist_size, cv::Size &size, cv::Size &area, int &padding);

void plot_size(int hist_size, cv::Size &size);

}
