#include "AnalizadorBgSubtractor.h"
#include <QDebug>

using namespace std;
using namespace cv;

AnalizadorBgSubtractor::~AnalizadorBgSubtractor() {
    if (_mask_out.isOpened())
        _mask_out.release();
    if (_model_out.isOpened())
        _model_out.release();
}
// -------------------------------------------------------------------
AlgoritmoSegm AnalizadorBgSubtractor::algoritmo() {
    return _algoritmo;
}
// -------------------------------------------------------------------
void AnalizadorBgSubtractor::set_algoritmo_segm(AlgoritmoSegm algoritmo) {
    _algoritmo = algoritmo;
}
// -------------------------------------------------------------------
void AnalizadorBgSubtractor::crear_bgSubtractor() {
    switch(_algoritmo) {
    case AlgoritmoSegm::KNN:
        _bgSub = createBackgroundSubtractorKNN();
        break;
    case AlgoritmoSegm::MOG2:
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
bool AnalizadorBgSubtractor::set_video(const string &path) {
    if (! Analizador::set_video(path, true))
        return false;

    string abs_path = build_abs_path(path);

    open_video_writer(_mask_out, outfilename(abs_path, "_mask.avi"), false);
    if (!_mask_out.isOpened()) {
        qCritical() << "set_video: Error creating mask output file";
        return false;
    }

    open_video_writer(_model_out, outfilename(abs_path, "_bg.avi"), true);
    if (!_model_out.isOpened()) {
        qCritical() << "set_video: Error creating model output file";
        return false;
    }

    return true;
}
// -------------------------------------------------------------------
void AnalizadorBgSubtractor::analizar_video() {
    assert(_video.isOpened());
    assert(_salida.isOpened());

    crear_bgSubtractor();
    assert(_bgSub != nullptr);

    qDebug() << "analizar_video: inicio";
    int i = 0;
    while (true) {
        _video >> _frame;

        if (i == 600 || _frame.empty()) {
            qDebug() << "analizar_video: fin " << i;
            break;
        }
        i++;

        if (i % 30 == 0) {
            qDebug() << "analizar_video: " << i/30 << " segundos analizados";
        }

        _bgSub->apply(_frame, _mask, -1.0);
        _mask_out << _mask;

        _bgSub->getBackgroundImage(_model);
        _model_out << _model;

        _frame.copyTo(_fg, _mask);
        _salida << _fg;
    }
    _salida.release();
    _video.release();
    _mask_out.release();
    _model_out.release();
}
// -------------------------------------------------------------------
std::string AnalizadorBgSubtractor::algoritmo_to_string(AlgoritmoSegm a) {
    switch(a) {
    case AlgoritmoSegm::KNN: return "KNN";
    case AlgoritmoSegm::MOG2: return "MOG2";
    }
    return "";
}
