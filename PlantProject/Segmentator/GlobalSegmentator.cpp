#include "GlobalSegmentator.h"
#include <qdebug.h>
#include <Utils.h>

using namespace std;
using namespace cv;

const std::map<GlobalSegmentator::ThreshType, double (*)(const Mat&, Mat&)> GlobalSegmentator::umbrs {
        { GlobalSegmentator::Medio,            &GlobalSegmentator::umbral_medio },
        { GlobalSegmentator::Fijo,             &GlobalSegmentator::umbral_fijo },
        { GlobalSegmentator::AdaptativoMedia,  &GlobalSegmentator::umbral_adaptativo_media },
        { GlobalSegmentator::AdaptativoGauss,  &GlobalSegmentator::umbral_adaptativo_gauss },
        { GlobalSegmentator::Otsu,             &GlobalSegmentator::umbral_otsu }
};
// -------------------------------------------------------------------
string GlobalSegmentator::umbralizado_to_str(ThreshType u)  {
    switch (u) {
    case Medio:           return "Medio";
    case Fijo:            return "Fijo";
    case AdaptativoMedia: return "Adaptativo de media";
    case AdaptativoGauss: return "Adaptativo gaussiano";
    case Otsu:            return "Binarización de Otsu";
    case ThreshTypeNum:
    default:
        return "Invalid enum";
    }
}
// -------------------------------------------------------------------
GlobalSegmentator::ThreshType GlobalSegmentator::thresh_type() const {
    return _conf.thresh_type;
}
// -------------------------------------------------------------------
double GlobalSegmentator::alpha() const {
    return _conf.alpha;
}
// -------------------------------------------------------------------
void GlobalSegmentator::process_2_frames() {
    assert(_video.isOpened());
    assert(_salida.isOpened());
    _ejemplo = true;

    // frame de noche
    _pos = NIGHT_SAMPLE_POS;
    _video.set(CAP_PROP_POS_FRAMES, _pos);
    _video >> _frame;
    process_frame();

    // frame de dia
    _pos = DAY_SAMPLE_POS;
    _video.set(CAP_PROP_POS_FRAMES, _pos);
    _video >> _frame;
    process_frame();

    _salida.release();
    _video.release();
}
// -------------------------------------------------------------------
void GlobalSegmentator::process_2_frames(cv::Mat &night, cv::Mat &day) {
    assert(_video.isOpened());
    assert(_salida.isOpened());
    _ejemplo = true;

    // frame de noche
    _pos = NIGHT_SAMPLE_POS;
    _video.set(CAP_PROP_POS_FRAMES, _pos);
    _video >> _frame;
    process_frame();
    night = _output.clone();

    // frame de dia
    _pos = static_cast<uint>(_video.get(CAP_PROP_FRAME_COUNT)) - 22*30;//DAY_SAMPLE_POS;
    _video.set(CAP_PROP_POS_FRAMES, _pos);
    _video >> _frame;
    process_frame();
    day = _output.clone();

    _salida.release();
    _video.release();
}
// -------------------------------------------------------------------
void GlobalSegmentator::process_frame() {
    preprocess();

    if (_ejemplo) {
        common::save_image(_wd, _frame, "umbr", to_string(_pos) + "-pr");
        qDebug() << "analizar_frame: processed frame";
    }

    umbralizar();
//    invert(_output, _output);
//    common::crop_time_bar(_output);

    if (_ejemplo) {
        common::save_image(_wd, _output, "umbr", to_string(_pos) + "-mask");
        qDebug() << "analizar_frame: threshold applied";
    }
}
// -------------------------------------------------------------------
void GlobalSegmentator::show_frame(const Mat &frame, const string &name) {
    if (_ejemplo) {
        Segmentator::show_frame(frame, name);
    }
}
// -------------------------------------------------------------------
// -------------------------- PREPROCESADO ---------------------------
// -------------------------------------------------------------------
void GlobalSegmentator::preprocess() {
    // Recortar imagen
    common::crop_roi(_frame);

    // A escala de grises
    cvtColor(_frame, _frame, COLOR_RGB2GRAY);
    if (_ejemplo) qDebug() << "preprocesar: to grayscale";

    // Aumentar contraste
    _frame *= _conf.alpha;
    if (_ejemplo) qDebug() << "preprocesar: contrast fix alpha ="
                           << _conf.alpha;
}
// -------------------------------------------------------------------
// -------------------------- UMBRALIZADO ----------------------------
// -------------------------------------------------------------------
void GlobalSegmentator::umbralizar() {
    double thr = umbrs.at(_conf.thresh_type)(_frame, _output);

    if (_ejemplo && (_conf.thresh_type == Medio
                     || _conf.thresh_type == Otsu))
        qDebug() << "umbralizar: thr =" << thr;
}
// -------------------------------------------------------------------
double GlobalSegmentator::umbral_fijo(const Mat &src, Mat &dst) {    
    double thr = 132.526;
    threshold(src, dst, thr, 255, THRESH_BINARY);
    return thr;
}
// -------------------------------------------------------------------
double GlobalSegmentator::umbral_medio(const Mat &src, Mat &dst) {
    double minVal, maxVal;

    common::crop_time_bar(src, dst);

    minMaxLoc(dst, &minVal, &maxVal);
    double thr = (maxVal + minVal) / 2;

    threshold(dst, dst, thr, 255, THRESH_BINARY);
    return thr;
}
// -------------------------------------------------------------------
double GlobalSegmentator::umbral_adaptativo_media(const Mat &src, Mat &dst) {
    adaptiveThreshold(src, dst, 255, ADAPTIVE_THRESH_MEAN_C,
                      THRESH_BINARY, 21, 6);
    return 0.;
}
// -------------------------------------------------------------------
double GlobalSegmentator::umbral_adaptativo_gauss(const Mat &src, Mat &dst) {
    adaptiveThreshold(src, dst, 255, ADAPTIVE_THRESH_GAUSSIAN_C,
                      THRESH_BINARY, 15, 2);
    return 0.;
}
// -------------------------------------------------------------------
double GlobalSegmentator::umbral_otsu(const Mat &src, Mat &dst) {
    common::crop_time_bar(src, dst);

    double thr = threshold(dst, dst, 0, 255, THRESH_OTSU);
    return thr;
}
