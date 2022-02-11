#include "analizadorframes.h"

using namespace std;
using namespace cv;

// -------------------------------------------------------------------
void AnalizadorFrames::analizar_2_frames() {
    assert(_video.isOpened());
    _mostrar = true;

    // _frame de noche
    // video.set(CAP_PROP_POS_MSEC, 10000);
    _video >> _frame;
    analizar_frame("night__frame");

    // _frame de dia
    // video.set(CAP_PROP_POS_MSEC, 15000);
    _video.set(CAP_PROP_POS_MSEC, 35000);
    _video >> _frame;
    analizar_frame("day__frame");

    _salida.release();
    _video.release();
}
// -------------------------------------------------------------------
void AnalizadorFrames::analizar_video() {
    assert(_video.isOpened());
    assert(_salida.isOpened());

    _mostrar = false;
    int i = 0;

    while (true) {
        _video >> _frame;

        if (_frame.empty() /*|| i == 600*/) break;

        if ((i + 1) % 150 == 0) {
            cout << (i + 1) / 30 << " segundos analizados." << endl;
            //_mostrar = true;
        } //else _mostrar = false;

        analizar_frame(to_string(i));

        _salida.write(_frame);
        i++;
    }
    cout << "Total _frames: " << i-1 << endl;

    _salida.release();
    _video.release();
}
// -------------------------------------------------------------------
void AnalizadorFrames::analizar_frame(string name) {
    mostrar_frame(_frame, name);

    preprocesar();
    mostrar_frame(_frame, name.append("-pro"));

    umbralizar();
    mostrar_frame(_frame, name.append("-umbral"));
}
// -------------------------------------------------------------------
void AnalizadorFrames::mostrar_frame(Mat frame, string name) {
    if (_mostrar) {
        namedWindow(name, WINDOW_NORMAL);
        moveWindow(name, 0, 0);
        resizeWindow(name, frame.size().width, frame.size().height);
        imshow(name, frame);
    }
}

// -------------------------------------------------------------------
// -------------------------- PREPROCESADO ---------------------------
// -------------------------------------------------------------------

void AnalizadorFrames::preprocesar() {
    // frame.convertTo(frame, CV_8UC1, 2);

    // A escala de grises
    cvtColor(_frame, _frame, COLOR_RGB2GRAY);

    // Aumentar contraste
    _frame *= _contraste;
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
    if (_mostrar)
        cout << "Umbral: " << umbral << endl;
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

    if (_mostrar) {
        cout << "Minimo: "<< minVal <<  " Maximo: " << maxVal;
        cout << " Umbral: " << umbral << endl;
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
    double thr = threshold(_frame, _frame, 0, 255, THRESH_OTSU);
    if (_mostrar)
        cout << "Umbral: " << thr << endl;
}
// -------------------------------------------------------------------
// --------------------------- MORFOLOGÍA ----------------------------
// -------------------------------------------------------------------
void AnalizadorFrames::invertir() {
    _frame = 255 - _frame;
}
// -------------------------------------------------------------------
void AnalizadorFrames::abrir() {
    Mat kernel = getStructuringElement(MORPH_RECT, Size(3, 3));
    morphologyEx(_frame, _frame, MORPH_OPEN, kernel);
}
