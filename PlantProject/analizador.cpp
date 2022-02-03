#include "analizador.h"
#include "stdio.h"
#include <windows.h>
#include <string>

using namespace std;
using namespace cv;

static VideoCapture video;
static VideoWriter salida;
static Mat frame;

static bool mostrar = false;

bool set_video(string path) {
    cout << path << endl;
    if (video.isOpened())
        video.release();

    video.open(path);

    if (salida.isOpened())
        salida.release();

    salida.open(outfilename(path), static_cast<int>(video.get(CAP_PROP_FOURCC)),
                video.get(CAP_PROP_FPS),
                Size(static_cast<int>(video.get(CAP_PROP_FRAME_WIDTH)),
                     static_cast<int>(video.get(CAP_PROP_FRAME_HEIGHT))));

    return video.isOpened() && salida.isOpened();
}

string outfilename(const string& filename) {
    size_t lastdot = filename.find_last_of(".");
    if (lastdot == string::npos) return filename;
    return filename.substr(0, lastdot).append("_thr.avi");
}

void show_video() {
    Mat frame;

    if (! video.isOpened()) return;

    namedWindow("Video", WINDOW_NORMAL);
    moveWindow("Video", 10, 10);

    while (true) {
        video >> frame;

        if (frame.empty()) {
            break;
        }

        imshow("Video", frame);
        Sleep(1000);
    }
}

void analizar_2_frames() {
    mostrar = true;

    // frame de noche
    video.set(CAP_PROP_POS_MSEC, 10000);
    video >> frame;
    analizar_frame("night_frame");

    // frame de dia
    video.set(CAP_PROP_POS_MSEC, 15000);
    video >> frame;
    analizar_frame("day_frame");

    salida.release();
    video.release();
}

void analizar_video() {
    mostrar = false;
    // video.set(CAP_PROP_POS_MSEC, 10000);
    int i = 0;

    while (true) {
        video >> frame;

        if (frame.empty() || i == 600) {
            break;
        }

        if ((i + 1) % 150 == 0) {
            cout << (i + 1) / 30 << " segundos analizados." << endl;
            mostrar = true;
        } else mostrar = false;

        analizar_frame(to_string(i));

        salida.write(frame);
        i++;
    }
    cout << "Total frames: " << i-1 << endl;

    salida.release();
    video.release();
}

void analizar_frame(string name) {
    // frame.convertTo(frame, CV_8UC1, 2);
    // A escala de grises
    cvtColor(frame, frame, COLOR_RGB2GRAY);
    // Aumenta contraste
    frame *= 2;

    if (mostrar) {
        namedWindow(name, WINDOW_NORMAL);
        moveWindow(name, 10, 0);
        resizeWindow(name, frame.size().width, frame.size().height);
        imshow(name, frame);
    }

    umbralizar_frame();
    name.append("-umbral");

    if (mostrar) {
        namedWindow(name, WINDOW_NORMAL);
        moveWindow(name, 540, 0);
        resizeWindow(name, frame.size().width, frame.size().height);
        imshow(name, frame);
    }
}

void umbralizar_frame() {
    // Umbral fijo con valor medio
    // umbral_medio(frame);

    // Umbral fijo
    umbral_fijo();

    // Umbral adaptativo media
    // adaptiveThreshold(frame, frame, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 7, 2);
}

void umbral_medio() {
    double minVal, maxVal;

    // Máscara para descartar el texto inferior
    Mat mask(frame.size(), CV_8UC1, Scalar(0));
    Rect rec(0, 0, frame.size().width, 645);
    rectangle(mask, rec, Scalar(255), FILLED);

    // Calcular umbral como media entre valores máximo mínimo de gris
    minMaxLoc(frame, &minVal, &maxVal, nullptr, nullptr, mask);
    double umbral = (maxVal + minVal) / 2;

    if (mostrar)
        cout << "Minimo: "<< minVal <<  " Maximo: " << maxVal << " Umbral: " << umbral << endl;

    threshold(frame, frame, umbral, 255, THRESH_BINARY);
}

void umbral_fijo() {
    double umbral = 132.526;
    if (mostrar)
        cout << "Umbral: " << umbral << endl;
    threshold(frame, frame, umbral, 255, THRESH_BINARY);
}
