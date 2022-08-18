#include "BgSubtractorSegmentator.h"
#include <QDebug>
#include <Utils.h>

using namespace std;
using namespace cv;

string BgSubtractorSegmentator::algorithm_to_string(Algorithm a) {
    switch(a) {
    case Algorithm::KNN: return "KNN";
    case Algorithm::MOG2: return "MOG2";
    case Algorithm::AlgorithmNum: return "";
    }
    return "";
}
// -------------------------------------------------------------------
BgSubtractorSegmentator::~BgSubtractorSegmentator() {
    if (_model_out.isOpened())
        _model_out.release();
}
// -------------------------------------------------------------------
void BgSubtractorSegmentator::create_bgSubtractor() {
    switch(_alg) {
    case Algorithm::KNN:
        _bgSub = createBackgroundSubtractorKNN();
        break;
    case Algorithm::MOG2:
        _bgSub = createBackgroundSubtractorMOG2();
        break;
    case Algorithm::AlgorithmNum:
    default:
        break;
    }
    if (_bgSub != nullptr)
        qDebug() << "crear_bgSubtractor: creado";
    else {
        qCritical() << "crear_bgSubtractor: no creado";
    }
}
// -------------------------------------------------------------------
bool BgSubtractorSegmentator::open_video_outputs(const std::string &path) {
    if (! Segmentator::open_video_outputs(path))
        return false;

    open_video_writer(_model_out, common::outfilename(path, "_model.avi"), true);
    if (!_model_out.isOpened()) {
        qCritical() << "set_video: Error creating model output file";
        return false;
    }

    return true;
}
// -------------------------------------------------------------------
void BgSubtractorSegmentator::set_up() {
    create_bgSubtractor();
    assert(_bgSub != nullptr);
}
// -------------------------------------------------------------------
void BgSubtractorSegmentator::process_frame() {
    _bgSub->apply(_frame, _output, -1.);
    _bgSub->getBackgroundImage(_model);

    common::crop_time_bar(_model);
    common::crop_time_bar(_output);

    _model_out << _model;
}
