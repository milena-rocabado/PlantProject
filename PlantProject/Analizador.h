#ifndef ANALIZADOR_H
#define ANALIZADOR_H

#include <string>
#include <opencv2/opencv.hpp>

class Analizador {
public:
    static constexpr int NIGHT_SAMPLE_POS = 30;
    static constexpr int DAY_SAMPLE_POS = 1050;

    static std::string wd();
    static void set_wd(const std::string& wd);

    virtual bool set_video(const std::string& path);
    void show_video();

    virtual void analizar_video() {}
    virtual ~Analizador();

protected:
    static cv::VideoCapture _video;
    static cv::VideoWriter _salida;
    static cv::Mat _frame;
    static std::string _wd;

    bool set_video(const std::string &path, const bool &isColor);
    bool open_video_writer(cv::VideoWriter &, const std::string &file, const bool &isColor);
    virtual void mostrar_frame(const cv::Mat &frame, const std::string &name);
    std::string outfilename(const std::string &filename, const std::string &suffix);
    std::string build_abs_path(const std::string &path);
    void save_image(const cv::Mat &im, const std::string &dir, const std::string &filename);
    void apply_mask(const cv::Mat &img, const cv::Mat &mask, cv::Mat &dst);
};

#endif // ANALIZADOR_H
