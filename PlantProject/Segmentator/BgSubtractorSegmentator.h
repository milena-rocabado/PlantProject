#ifndef ANALIZADORBGSUBTRACTOR_H
#define ANALIZADORBGSUBTRACTOR_H

#include <Segmentator.h>
#include <opencv2/video/background_segm.hpp>
#include <QMetaType>

class BgSubtractorSegemntator : public Segmentator
{
public:
    enum Algorithm {
        KNN,
        MOG2
    };

    BgSubtractorSegemntator():
        _alg(KNN)
    {}
    ~BgSubtractorSegemntator() override;

    Algorithm alg() { return _alg; }
    void set_alg(const Algorithm &alg) { _alg = alg; }

    bool set_video(const std::string &path) override;
    void process_debug() override { process_video(); }

    static std::string algorithm_to_string(Algorithm a) {
        switch(a) {
        case Algorithm::KNN: return "KNN";
        case Algorithm::MOG2: return "MOG2";
        }
        return "";
    }

protected:
    void set_up() override;
    void process_frame() override;

private:
    Algorithm _alg;

    cv::Ptr<cv::BackgroundSubtractor> _bgSub;
    cv::Mat _model;

    cv::VideoWriter _model_out;

    void create_bgSubtractor();
};

#endif // ANALIZADORBGSUBTRACTOR_H
