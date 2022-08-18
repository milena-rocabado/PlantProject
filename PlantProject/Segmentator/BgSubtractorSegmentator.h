#ifndef ANALIZADORBGSUBTRACTOR_H
#define ANALIZADORBGSUBTRACTOR_H

#include <Segmentator.h>
#include <opencv2/video/background_segm.hpp>
#include <QMetaType>

class BgSubtractorSegmentator : public Segmentator
{
public:
    enum Algorithm {
        KNN,
        MOG2,
        AlgorithmNum
    };

    static constexpr Algorithm ALG_DF = KNN;

    BgSubtractorSegmentator():
        _alg(KNN)
    {}

    BgSubtractorSegmentator(Algorithm alg):
        _alg(alg)
    {}

    ~BgSubtractorSegmentator() override;

    Algorithm alg() { return _alg; }
    void set_alg(const Algorithm &alg) { _alg = alg; }

    void process_debug() override { process_video(); }

    static std::string algorithm_to_string(Algorithm a);

protected:
    bool open_video_outputs(const std::string &path) override;
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
