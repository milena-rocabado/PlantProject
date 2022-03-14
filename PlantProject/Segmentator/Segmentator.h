#ifndef ANALIZADOR_H
#define ANALIZADOR_H

#include <string>
#include <opencv2/opencv.hpp>
#include <chrono>

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))

class Segmentator {
public:
    static constexpr int NIGHT_SAMPLE_POS = 30;
    static constexpr int DAY_SAMPLE_POS = 1050;

    static constexpr int BAR_HEIGHT = 20;

    virtual bool set_video(const std::string& path);
    void show_video();

    void process_timed();
    void process_video();
    virtual void process_debug() = 0;

    virtual void set_up() = 0;
    virtual void process_frame() = 0;

    virtual ~Segmentator();

    double time() { return _time.count(); }

protected:
    uint _pos;
    cv::VideoCapture _video;
    cv::VideoWriter _salida;
    cv::Mat _frame;
    cv::Mat _resultado;
    std::string _wd;

    bool _ejemplo;

    bool set_video(const std::string &path, const bool &isColor, const bool &isCropped);
    bool open_video_writer(cv::VideoWriter &, const std::string &file, const bool &isColor, const bool &isCropped);
    std::string outfilename(const std::string &filename, const std::string &suffix);

    virtual void show_frame(const cv::Mat &frame, const std::string &name);
    void save_image(const cv::Mat &im, const std::string &dir, const std::string &filename);
    void save_image(const cv::Mat &im, const std::string &filename);

    static void apply_mask(const cv::Mat &img, const cv::Mat &mask, cv::Mat &dst);
    static void crop_time_bar(cv::Mat &);
    static void crop_time_bar(const cv::Mat &src, cv::Mat &dst);
    static void abrir(cv::Mat &);
    static void invertir(cv::Mat &);

private:
    std::chrono::high_resolution_clock::time_point _start;
    std::chrono::duration<double> _time;

    inline void start_timer() {
        _start = std::chrono::high_resolution_clock::now();
    }
    inline void stop_timer() {
        auto end= std::chrono::high_resolution_clock::now();
        _time = end - _start;
    }

    bool get_wd_from(const std::string &path);
    std::string build_abs_path(const std::string &path);
};

#endif // ANALIZADOR_H
