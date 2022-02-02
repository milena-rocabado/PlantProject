#include "analizador.h"
#include "stdio.h"
#include <windows.h>
#include <string>

using namespace std;
using namespace cv;

static VideoCapture video;
static VideoWriter salida;

static bool mostrar;
static double umbral_suma = 0.;

bool set_video(string path) {
    cout << path << endl;
    if (video.isOpened())
        video.release();

    video.open(path);

//    if (salida.isOpened())
//        salida.release();

//    salida.open(outfilename(path), VideoWriter::fourcc('D','I','V','X'),
//                video.get(CAP_PROP_FPS),
//                Size(static_cast<int>(video.get(CAP_PROP_FRAME_WIDTH)),
//                     static_cast<int>(video.get(CAP_PROP_FRAME_HEIGHT))));

    return video.isOpened(); // && salida.isOpened();
}

string outfilename(const string& filename) {
    size_t lastdot = filename.find_last_of(".");
    if (lastdot == string::npos) return filename;
    return filename.substr(0, lastdot).append("_thr.avi");
}

void show_video() {
    int i = 0;
    Mat img;

    if (! video.isOpened()) return;

    namedWindow("Video", WINDOW_NORMAL);
    moveWindow("Video", 10, 10);

    while (true) {
        i++;
        video >> img;

        if (img.empty()) {
            cout << "Last frame " << i - 1 << endl;
            break;
        }

        imshow("Video", img);
        Sleep(1000);
    }

    destroyAllWindows();
}

void analizar_2_frames() {
    mostrar = true;

    // frame de noche
    video.set(CAP_PROP_POS_MSEC, 10000);
    Mat img_n;
    video >> img_n;
    analizar_frame(img_n, "night_frame");

    // frame de dia
    video.set(CAP_PROP_POS_MSEC, 15000);
    Mat img_d;
    video >> img_d;
    analizar_frame(img_d, "day_frame");

    // salida.release();
    video.release();
}

void analizar_video() {
    mostrar = false;
    // video.set(CAP_PROP_POS_MSEC, 10000);
    int i = 0, num_medias = 0;
    double medias = 0., umbral_medio;
    Mat frame;

    while (true) {
        video >> frame;

        if (frame.empty()) {
            umbral_medio = medias / num_medias;
            cout << "Umbral medio: " << umbral_medio << endl;
            break;
        }

        analizar_frame(frame, to_string(i));
        // salida << frame;

        if ((i + 1) % 300 == 0) {
            double media_parcial = umbral_suma / 300;
            medias += media_parcial;
            num_medias++;
            umbral_suma = 0.;
            cout << "(" << (i + 1) / 300 << ") Media parcial: " << media_parcial << endl;
        }

        i++;
    }
    cout << "Total frames: " << i-1 << endl;

    // salida.release();
    video.release();
}

void analizar_frame(Mat img, string name) {
    // A escala de grises
    cvtColor(img, img, COLOR_RGB2GRAY);
    // Aumenta contraste
    img *= 2;

    if (mostrar) {
        namedWindow(name, WINDOW_NORMAL);
        moveWindow(name, 10, 0);
        imshow(name, img);
    }

    umbralizar_frame(img);
    name.append("2");

    if (mostrar) {
        namedWindow(name, WINDOW_NORMAL);
        moveWindow(name, 2000, 0);
        imshow(name, img);
    }
}

void umbralizar_frame(Mat img) {
    // Umbral fijo con valor medio
    // umbral_medio(img);

    // Umbral fijo
    umbral_fijo(img);

    // Umbral adaptativo media
    // adaptiveThreshold(img, img, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 7, 2);
}

void umbral_medio(cv::Mat img) {
    double minVal, maxVal;

    // Máscara para descartar el texto inferior
    Mat mask(img.size(), CV_8UC1, Scalar(0));
    Rect rec(0, 0, img.size().width, 645);
    rectangle(mask, rec, Scalar(255), FILLED);

    // Calcular umbral como media entre valores máximo mínimo de gris
    minMaxLoc(img, &minVal, &maxVal, nullptr, nullptr, mask);
    double umbral = (maxVal + minVal) / 2;

    if (mostrar)
        cout << "Minimo: "<< minVal <<  " Maximo: "<< maxVal << " Umbral: " << umbral << endl;

    umbral_suma += umbral;

    threshold(img, img, umbral, 255, THRESH_BINARY);
}

void umbral_fijo(cv::Mat img) {
    double umbral = 132.526;
    threshold(img, img, umbral, 255, THRESH_BINARY);
}
