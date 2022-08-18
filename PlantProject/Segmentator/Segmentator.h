#ifndef ANALIZADOR_H
#define ANALIZADOR_H

#include <string>
#include <opencv2/opencv.hpp>
#include <chrono>


class Segmentator {
public:
    static constexpr int NIGHT_SAMPLE_POS { 30 };
    static constexpr int DAY_SAMPLE_POS { 1050 };

    Segmentator()
        : _pos(0)
    {}

    bool set_video(const std::string& path);
    void set_wd(const std::string &path);
    virtual void set_up() = 0;

    void show_video();

    void process_timed();
    void process_video();
    virtual void process_debug() = 0;

    void process_frame(const cv::Mat &frame, cv::Mat &output);

    virtual ~Segmentator();

    double time() { return _time.count(); }

protected:
    uint _pos;
    cv::VideoCapture _video;
    cv::VideoWriter _salida;
    cv::Mat _frame;
    cv::Mat _output;
    std::string _wd;

    bool _ejemplo;

    virtual bool open_video_outputs(const std::string &path);
    bool open_video_writer(cv::VideoWriter &, const std::string &file, const bool &isColor);

    virtual void process_frame() = 0;
    virtual void show_frame(const cv::Mat &frame, const std::string &name);
    static void open(const cv::Mat &src, cv::Mat &dst);
    static void invert(const cv::Mat &src, cv::Mat &dst);

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
};

#endif // ANALIZADOR_H
