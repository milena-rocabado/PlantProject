#include "AdaptiveModelSegmentator.h"
#include <QtDebug>
#include <Utils.h>

using namespace std;
using namespace cv;

const uint AdaptiveModelSegmentator::DN_BREAKPOINTS[];

// -------------------------------------------------------------------
AdaptiveModelSegmentator::~AdaptiveModelSegmentator() {
    _night_model.release();
    _day_model.release();
}
// -------------------------------------------------------------------
double AdaptiveModelSegmentator::alpha() {
    return _alpha;
}
// -------------------------------------------------------------------
void AdaptiveModelSegmentator::set_alpha(const double &contraste) {
    _alpha = contraste;
    _apply_contrast = static_cast<int>(_alpha * 100) != 100;
}
// -------------------------------------------------------------------
void AdaptiveModelSegmentator::set_up() {
    qDebug() << "set_up: attempting to open " << QString::fromStdString(_wd + "data.txt");
    _file.open(_wd + "data.txt");

    if (! _file.is_open())
        qCritical() << "set_up: failed to open data output text file";

    load_model(true);
    load_model(false);
}
// -------------------------------------------------------------------
void AdaptiveModelSegmentator::load_model(const bool &noche) {
    string fp = _wd + (noche ? "modelo_noche.png" : "modelo_dia.png");
    qDebug() << "cargar_modelos: attempting to load model"
             << QString::fromStdString(fp);

    Mat *modelo = (noche ? &_night_model : &_day_model);

    *modelo = imread(fp);
    if (modelo->empty()) {
        qDebug() << "cargar_modelos: failed to load night model";
        _night = noche;
        create_model();
    } else qDebug() << "cargar_modelos: success loading model";

    cvtColor(*modelo, *modelo, COLOR_BGR2GRAY);
    qDebug() << "cargar_modelos: model converted to grayscale";
    common::crop_time_bar(*modelo);
    qDebug() << "cargar_modelos: time bar cropped model";
}
// -------------------------------------------------------------------
void AdaptiveModelSegmentator::create_model() {
    assert(_video.isOpened());

    Mat modelo(static_cast<int>(_video.get(CAP_PROP_FRAME_HEIGHT)),
               static_cast<int>(_video.get(CAP_PROP_FRAME_WIDTH)),
               CV_32FC3,
               Scalar(0, 0, 0));
    int num_frames = 0;
    uint i, j = _night ? 0 : 1;

    qDebug() << "calcular_modelo: start = 0";
    for (i = 0; ; i++) {
        if (i == DN_BREAKPOINTS[j]) {
            if (++j == ARRAY_SIZE(DN_BREAKPOINTS)) {
                qDebug() << "calcular_modelo: end of array";
                break;
            }

            qDebug() << "calcular_modelo: before POS_FRAMES =" << i
                     << "j =" << j;
            _video.set(CAP_PROP_POS_FRAMES, DN_BREAKPOINTS[j] + 1);
            i = DN_BREAKPOINTS[j++] + 1;
            qDebug() << "calcular_modelo:  after POS_FRAMES =" << i
                     << "j =" << j;
        }

        _video >> _frame;
        if (_frame.empty()) {
            qDebug() << "calcular_modelo: end of video";
            break;
        }

        _frame.convertTo(_frame, CV_32F);
        modelo += _frame;
        num_frames++;
    }
    qDebug() << "calcular_modelo: total =" << num_frames;

    modelo = modelo / num_frames;
    modelo.convertTo(_night ? _night_model : _day_model, CV_8U);

    string n = _night ? "noche.png" : "dia.png";
    string fn = _wd + "modelo_" + n;

    qDebug() << "calcular_modelo: saving model to" << QString::fromStdString(fn);
    imwrite(fn, modelo);
    _video.set(CAP_PROP_POS_FRAMES, 0);
}
// -------------------------------------------------------------------
void AdaptiveModelSegmentator::process_frame() {
    common::crop_time_bar(_frame);
    cvtColor(_frame, _frame, COLOR_BGR2GRAY);
    if (_ejemplo) qDebug() << "analizar_frame: time bar cropped frame";

    Mat diferencia(_frame.size(), CV_8U);

    if (_night)
        diferencia = _night_model - _frame;
    else
        diferencia = _day_model - _frame;

    if (_ejemplo) {
        show_frame(diferencia, to_string(_pos) + "-dif");
        common::save_image(_wd, diferencia, "segmA", to_string(_pos) + "-dif");
        qDebug() << "analizar_frame: dif computed";
    }

    if (_apply_contrast) {
        diferencia *= _alpha;
        if (_ejemplo) show_frame(diferencia, to_string(_pos) + "-dif-cont");
    }

    threshold(diferencia, _output, 65.0, 255.0, THRESH_BINARY);
    if (_ejemplo) {
        show_frame(_output, to_string(_pos) + "-mask");
        common::save_image(_wd, _output, "segmA", to_string(_pos) + "-mask");
        qDebug() << "analizar_frame: mask computed";
    }

    _file << mean(_output)[0];
}
// -------------------------------------------------------------------
void AdaptiveModelSegmentator::process_2_frames() {
    qDebug() << "analizar_2_frames: start";
    load_model(true);
    load_model(false);
    _ejemplo = true;
    _night = true;

    _video.set(CAP_PROP_POS_FRAMES, NIGHT_SAMPLE_POS);
    _video >> _frame;
    _pos = NIGHT_SAMPLE_POS;
    iterarate_thresh();

    _night = false;
    _video.set(CAP_PROP_POS_FRAMES, DAY_SAMPLE_POS);
    _pos = DAY_SAMPLE_POS;
    _video >> _frame;
    iterarate_thresh();
}
// -------------------------------------------------------------------
void AdaptiveModelSegmentator::iterarate_thresh() {
    common::crop_time_bar(_frame);
    cvtColor(_frame, _frame, COLOR_BGR2GRAY);
    if (_ejemplo) qDebug() << "calcular_mascara: time bar cropped frame";

    Mat diferencia(_frame.size(), CV_8U);

    diferencia = (_night ? _night_model : _day_model) - _frame;
    qDebug() << "calcular_mascara: dif computed";

    if (_apply_contrast) {
        diferencia *= _alpha;
        common::save_image(_wd, diferencia, "segmA", to_string(_pos) + "-dif");
    }

    double start = 1., end = 80., increment = 20.;
    for (double i = start; i < end; i += increment)
    {
        threshold(diferencia, _output, i, 255.0, THRESH_BINARY);
        common::save_image(_wd, _output, "segmA/umb", to_string(_pos) + "-mask-" + to_string(i));
        qDebug() << "calcular_mascara: mask computed thr =" << i;
    }
    double thr = threshold(diferencia, _output, 0., 255., THRESH_OTSU);
    common::save_image(_wd, _output, "segmA/umb", to_string(_pos) + "-mask-otsu");
    qDebug() << "calcular_mascara: mask computed thr =" << thr;
}
