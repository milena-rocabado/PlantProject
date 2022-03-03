#include "AnalizadorFrames.h"
#include <qdebug.h>

using namespace std;
using namespace cv;

Umbralizado AnalizadorFrames::umbralizado() {
    return _umbralizado;
}
// -------------------------------------------------------------------
double AnalizadorFrames::contraste() {
    return _contraste;
}
// -------------------------------------------------------------------
bool AnalizadorFrames::mostrar() {
    return _ejemplo;
}
// -------------------------------------------------------------------
void AnalizadorFrames::set_umbralizado(const Umbralizado &umbralizado) {
    _umbralizado = umbralizado;
}
// -------------------------------------------------------------------
void AnalizadorFrames::set_contraste(const double &contraste) {
    _contraste = contraste;
}
// -------------------------------------------------------------------
void AnalizadorFrames::set_mostrar(const bool &mostrar) {
    _ejemplo = mostrar;
}
// ------------------------------------------------------------------
std::string AnalizadorFrames::umbralizado_to_string(Umbralizado u) {
   switch (u) {
   case Umbralizado::Medio:           return "Medio";
   case Umbralizado::Fijo:            return "Fijo";
   case Umbralizado::AdaptativoMedia: return "Adaptativo de media";
   case Umbralizado::AdaptativoGauss: return "Adaptativo gaussiano";
   case Umbralizado::Otsu:            return "Binarización de Otsu";
   }
   return "Invalid enum";
}
// -------------------------------------------------------------------
void AnalizadorFrames::analizar_2_frames() {
    assert(_video.isOpened());
    assert(_salida.isOpened());
    _ejemplo = true;

    // frame de noche
    // _video.set(CAP_PROP_POS_MSEC, 10000);
    _video.set(CAP_PROP_POS_FRAMES, DAY_SAMPLE_POS);

    _video >> _frame;
    analizar_frame("night_frame");

    // frame de dia
    // _video.set(CAP_PROP_POS_MSEC, 15000);
    // _video.set(CAP_PROP_POS_MSEC, 35000);
    _video.set(CAP_PROP_POS_FRAMES, NIGHT_SAMPLE_POS);
    _video >> _frame;
    analizar_frame("day_frame");

    _salida.release();
    _video.release();
}
// -------------------------------------------------------------------
void AnalizadorFrames::analizar_video() {
    assert(_video.isOpened());
    assert(_salida.isOpened());

    _ejemplo = false;
    _i = 0;

    while (true) {
        _video >> _frame;

        if (_i == 1051 || _frame.empty()) break;

        _ejemplo = _i == NIGHT_SAMPLE_POS || _i == DAY_SAMPLE_POS;

        analizar_frame(to_string(_i));

        _salida << _frame;
        _i++;
    }
    cout << "Total frames: " << _i-1 << endl;

    _salida.release();
    _video.release();
}
// -------------------------------------------------------------------
void AnalizadorFrames::analizar_frame(string name) {
    mostrar_frame(_frame, name);

    preprocesar();
    //mostrar_frame(_frame, name.append("-pro"));
    if (_ejemplo) save_image(_frame, "umbr", name + "-pr");

    umbralizar();
    mostrar_frame(_frame, name + "-umbral");
    if (_ejemplo) save_image(_frame, "umbr", name + "-mask");
}
// -------------------------------------------------------------------
void AnalizadorFrames::mostrar_frame(const Mat &frame, const string &name) {
    if (_ejemplo) {
        Analizador::mostrar_frame(frame, name);
    }
}
// -------------------------------------------------------------------
// -------------------------- PREPROCESADO ---------------------------
// -------------------------------------------------------------------
void AnalizadorFrames::preprocesar() {
    // A escala de grises
    cvtColor(_frame, _frame, COLOR_RGB2GRAY);
    if (_ejemplo) qDebug() << "preprocesar: to grayscale";

    // Aumentar contraste
    _frame *= _contraste;
    if (_ejemplo) qDebug() << "preprocesar: contrast fix alpha = "
                           << _contraste;
}
// -------------------------------------------------------------------
// -------------------------- UMBRALIZADO ----------------------------
// -------------------------------------------------------------------
void AnalizadorFrames::umbralizar() {
    switch(_umbralizado){
    case Umbralizado::Fijo:
        umbral_fijo();
        break;
    case Umbralizado::Medio:
        umbral_medio();
        break;
    case Umbralizado::AdaptativoMedia:
        umbral_adaptativo_media();
        break;
    case Umbralizado::AdaptativoGauss:
        umbral_adaptativo_gauss();
        break;
    case Umbralizado::Otsu:
        umbral_otsu();
        break;
    }
}
// -------------------------------------------------------------------
void AnalizadorFrames::umbral_fijo() {
    double umbral = 132.526;
    if (_ejemplo)
        qDebug() << "umbral_fijo: Umbral= " << umbral;
    threshold(_frame, _frame, umbral, 255, THRESH_BINARY);
}
// -------------------------------------------------------------------
void AnalizadorFrames::umbral_medio() {
    double minVal, maxVal;

    // Máscara para descartar el texto inferior
    Mat mask(_frame.size(), CV_8UC1, Scalar(0));
    Rect rec(0, 0, _frame.size().width, 645);
    rectangle(mask, rec, Scalar(255), FILLED);

    // Calcular umbral como media entre valores máximo mínimo de gris
    minMaxLoc(_frame, &minVal, &maxVal, nullptr, nullptr, mask);
    double umbral = (maxVal + minVal) / 2;

    if (_ejemplo) {
        qDebug() << "umbral_media: Minimo = "<< minVal <<  " Maximo = "
                 << maxVal << " Umbral = " << umbral;
    }

    threshold(_frame, _frame, umbral, 255, THRESH_BINARY);
}
// -------------------------------------------------------------------
void AnalizadorFrames::umbral_adaptativo_media() {
    adaptiveThreshold(_frame, _frame, 255, ADAPTIVE_THRESH_MEAN_C,
                      THRESH_BINARY, 21, 6);
}
// -------------------------------------------------------------------
void AnalizadorFrames::umbral_adaptativo_gauss() {
    adaptiveThreshold(_frame, _frame, 255, ADAPTIVE_THRESH_GAUSSIAN_C,
                      THRESH_BINARY, 15, 2);
}
// -------------------------------------------------------------------
void AnalizadorFrames::umbral_otsu() {
    // crop_time_bar(_frame);
    if (_ejemplo) {
        qDebug() << "umbral_otsu: time bar cropped";
        save_image(_frame, "umbr", to_string(_i)+"-cropped");
    }


    double thr = threshold(_frame, _frame, 0, 255, THRESH_OTSU);
    if (_ejemplo) qDebug() << "umbral_otsu: threshold = " << thr;
}
// -------------------------------------------------------------------
// --------------------------- MORFOLOGÍA ----------------------------
// -------------------------------------------------------------------
void AnalizadorFrames::invertir() {
    _frame = 255 - _frame;
}
