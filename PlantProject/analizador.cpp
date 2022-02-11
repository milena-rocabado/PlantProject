#include "analizador.h"
#include "stdio.h"
#include <windows.h>
#include <string>
#include <fstream>

using namespace std;
using namespace cv;

static VideoCapture video;
static VideoWriter salida;
static Mat frame;

static bool mostrar = false;

string outfilename(const string& filename);

bool set_video(string path) {
    cout << path << endl;
    if (video.isOpened())
        video.release();

    video.open(path);

    if (salida.isOpened())
        salida.release();

    salida.open(outfilename(path),
                static_cast<int>(video.get(CAP_PROP_FOURCC)),
                video.get(CAP_PROP_FPS),
                Size(static_cast<int>(video.get(CAP_PROP_FRAME_WIDTH)),
                     static_cast<int>(video.get(CAP_PROP_FRAME_HEIGHT))), false);

    return video.isOpened() && salida.isOpened();
}
// ------------------------------------------------------------------
string outfilename(const string& filename) {
    size_t lastdot = filename.find_last_of(".");
    if (lastdot == string::npos) return filename;
    return filename.substr(0, lastdot).append("_thr.avi");
}
// ------------------------------------------------------------------
void show_video() {
    assert(video.isOpened());
    Mat frame;
    unsigned long ms = static_cast<unsigned long>(1000/video.get(CAP_PROP_FPS));

    if (! video.isOpened()) return;

    namedWindow("Video", WINDOW_NORMAL);
    moveWindow("Video", 10, 10);

    while (true) {
        video >> frame;

        if (frame.empty()) {
            break;
        }

        imshow("Video", frame);
        Sleep(ms);
    }
}
// ------------------------------------------------------------------
void analizar_2_frames() {
    assert(video.isOpened());
    mostrar = true;

    // frame de noche
    // video.set(CAP_PROP_POS_MSEC, 10000);
    video >> frame;
    analizar_frame("night_frame");

    // frame de dia
    // video.set(CAP_PROP_POS_MSEC, 15000);
    video.set(CAP_PROP_POS_MSEC, 35000);
    video >> frame;
    analizar_frame("day_frame");

    salida.release();
    video.release();
}
// ------------------------------------------------------------------
void analizar_video() {
    assert(video.isOpened());
    assert(salida.isOpened());

    mostrar = false;
    int i = 0;

    while (true) {
        video >> frame;

        if (frame.empty() /*|| i == 600*/) break;

        if ((i + 1) % 150 == 0) {
            cout << (i + 1) / 30 << " segundos analizados." << endl;
            //mostrar = true;
        } //else mostrar = false;

        analizar_frame(to_string(i));

        salida.write(frame);
        i++;
    }
    cout << "Total frames: " << i-1 << endl;

    salida.release();
    video.release();
}
// ------------------------------------------------------------------
void analizar_frame(string name) {
    mostrar_frame(frame, name);

    preprocesar();
    mostrar_frame(frame, name.append("-pro"));

    umbralizar();
    mostrar_frame(frame, name.append("-umbral"));
}
// ------------------------------------------------------------------
void mostrar_frame(Mat frame, string name) {
    if (mostrar) {
        namedWindow(name, WINDOW_NORMAL);
        moveWindow(name, 0, 0);
        resizeWindow(name, frame.size().width, frame.size().height);
        imshow(name, frame);
    }
}

// ------------------------------------------------------------------
// -------------------------- PREPROCESADO --------------------------
// ------------------------------------------------------------------

void preprocesar() {
    // frame.convertTo(frame, CV_8UC1, 2);

    // A escala de grises
    cvtColor(frame, frame, COLOR_RGB2GRAY);

    // Aumentar contraste
    frame *= 2;
}

// ------------------------------------------------------------------
// -------------------------- UMBRALIZADO ---------------------------
// ------------------------------------------------------------------

void umbralizar() {
    // Umbral fijo con valor medio
    // umbral_medio(frame);

    // Umbral fijo
    // umbral_fijo();

    // Umbral adaptativo media
    // adaptiveThreshold(frame, frame, 255, ADAPTIVE_THRESH_MEAN_C,
    //                   THRESH_BINARY, 21, 6);

    // Umbral adaptativo gaussiano
    // adaptiveThreshold(frame, frame, 255, ADAPTIVE_THRESH_GAUSSIAN_C,
    //                   THRESH_BINARY, 15, 2);

    // Binarización de Otsu
    double thr = threshold(frame, frame, 0, 255, THRESH_OTSU);
    cout << "Umbral: " << thr << endl;
}
// ------------------------------------------------------------------
void umbral_medio() {
    double minVal, maxVal;

    // Máscara para descartar el texto inferior
    Mat mask(frame.size(), CV_8UC1, Scalar(0));
    Rect rec(0, 0, frame.size().width, 645);
    rectangle(mask, rec, Scalar(255), FILLED);

    // Calcular umbral como media entre valores máximo mínimo de gris
    minMaxLoc(frame, &minVal, &maxVal, nullptr, nullptr, mask);
    double umbral = (maxVal + minVal) / 2;

    if (mostrar) {
        cout << "Minimo: "<< minVal <<  " Maximo: " << maxVal;
        cout << " Umbral: " << umbral << endl;
    }

    threshold(frame, frame, umbral, 255, THRESH_BINARY);
}
// ------------------------------------------------------------------
void umbral_fijo() {
    double umbral = 132.526;
    if (mostrar)
        cout << "Umbral: " << umbral << endl;
    threshold(frame, frame, umbral, 255, THRESH_BINARY);
}

// ------------------------------------------------------------------
// --------------------------- MORFOLOGÍA ---------------------------
// ------------------------------------------------------------------

void invertir() {
    frame = 255 - frame;
}
// -------------------------------------------------------------------
void abrir() {
    Mat kernel = getStructuringElement(MORPH_RECT, Size(3, 3));
    morphologyEx(frame, frame, MORPH_OPEN, kernel);
}
