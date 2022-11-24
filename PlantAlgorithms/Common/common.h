#ifndef COMMON_H
#define COMMON_H

#include <opencv2/opencv.hpp>
#include <cstdio>

#define HIST_SIZE 256

#define BAR_HEIGHT 20
#define POT_Y_POS 567

#define NIGHT_SAMPLE 30
#define DAY_SAMPLE 1050

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))

// PLATFORM CONSTANTS ----------------------------------------------------------
#ifdef DESKTOP
#define WD "C:/Users/radsa/git/TFG-PlantProject/Media/"
#define DUMP_WD "C:/Users/radsa/git/TFG-PlantProject/Media/Ejemplos/"
#else
#define WD "C:/Users/milena/git/PlantProject/Media/"
#define DUMP_WD "C:/Users/milena/git/PlantProject/Media/Ejemplos/"
#endif

// DEBUG MACROS ----------------------------------------------------------------
#ifdef DEBUG
#define TRACE(b, ...) { if (b) fprintf (stderr, __VA_ARGS__); }
#define DUMP(b, mat_, ...) { if (b) { \
    char buf_[256]; \
    std::sprintf(buf_, __VA_ARGS__); \
    imwrite(std::string(DUMP_WD) + std::string(buf_), mat_); \
    } \
}
#define DUMP_HIST(b, hist_, ...) { if (b) { \
    cv::Mat histogram_plot_; \
    plot_hist(hist_, histogram_plot_); \
    char buf_[256]; \
    std::sprintf(buf_, __VA_ARGS__); \
    imwrite(std::string(DUMP_WD) + std::string(buf_), histogram_plot_); \
    } \
}
#define DUMP_HIST_N(b, hist_, n_, ...) { if (b) { \
    cv::Mat histogram_plot_; \
    plot_hist(hist_, histogram_plot_, n_); \
    char buf_[256]; \
    std::sprintf(buf_, __VA_ARGS__); \
    imwrite(std::string(DUMP_WD) + std::string(buf_), histogram_plot_); \
    } \
}
#else
#define TRACE(b, ...) ;
#define DUMP(b, name, mat) ;
#define DUMP_HIST(b, hist_, ...) ;
#define DUMP_HIST_N(b, hist_, ...) ;
#endif

static int DN_BREAKPOINTS[] = { 436, 1395, 1877, 2836, 3314, 4188 };
                           // { 436, 1395, 1877, 2836, 3314, 4189 };

static std::string wd { WD };

// MODIFY ----------------------------------------------------------------------
void crop_roi(const cv::Mat &src, cv::Mat &dst);
void crop_upper_third(const cv::Mat &src, cv::Mat &dst);

// IO --------------------------------------------------------------------------
void save(const std::string &fn, const cv::Mat &img);
void show(const std::string &name, const cv::Mat &img);
std::string image_type(const cv::Mat &img);

// HISTOGRAMS ------------------------------------------------------------------
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

void plot_hist(const cv::Mat &hist, cv::Mat &output, int v_line = -1);

void vertical_line(cv::Mat &img, int pos);

void get_plot_size(int hist_size, cv::Size &size, cv::Size &area, int &padding);

void get_plot_size(int hist_size, cv::Size &size);
// -----------------------------------------------------------------------------
template <typename T>
extern void plot_vector(const std::vector<T> v, cv::Mat &plot);

#endif // COMMON_H
