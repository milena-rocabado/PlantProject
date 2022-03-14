#include "BgSubtractorSegmentator.h"
#include <QDebug>

using namespace std;
using namespace cv;

BgSubtractorSegemntator::~BgSubtractorSegemntator() {
    if (_model_out.isOpened())
        _model_out.release();
}
// -------------------------------------------------------------------
void BgSubtractorSegemntator::create_bgSubtractor() {
    switch(_alg) {
    case Algorithm::KNN:
        _bgSub = createBackgroundSubtractorKNN();
        break;
    case Algorithm::MOG2:
        _bgSub = createBackgroundSubtractorMOG2();
        break;
    }
    if (_bgSub != nullptr)
        qDebug() << "crear_bgSubtractor: creado";
    else {
        qCritical() << "crear_bgSubtractor: no creado";
    }
}
// -------------------------------------------------------------------
bool BgSubtractorSegemntator::set_video(const string &path) {
    if (! Segmentator::set_video(path, false, false))
        return false;

    open_video_writer(_model_out, outfilename(path, "_bg.avi"),true, false);
    if (!_model_out.isOpened()) {
        qCritical() << "set_video: Error creating model output file";
        return false;
    }

    return true;
}
// -------------------------------------------------------------------
void BgSubtractorSegemntator::set_up() {
    create_bgSubtractor();
    assert(_bgSub != nullptr);
}
// -------------------------------------------------------------------
void BgSubtractorSegemntator::process_frame() {
    _bgSub->apply(_frame, _resultado, -1.);
    _bgSub->getBackgroundImage(_model);
    _model_out << _model;
}
