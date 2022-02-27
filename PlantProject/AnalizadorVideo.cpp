#include "AnalizadorVideo.h"
#include <QtDebug>

using namespace std;
using namespace cv;

AnalizadorVideo::~AnalizadorVideo() {
    _modelo.release();
    _mascara.release();
}
// ------------------------------------------------------------------
bool AnalizadorVideo::adaptativo() {
    return _adaptativo;
}
// ------------------------------------------------------------------
double AnalizadorVideo::contraste() {
    return _contraste;
}
// ------------------------------------------------------------------
int AnalizadorVideo::num_frames() {
    return _num_frames;
}
// ------------------------------------------------------------------
int AnalizadorVideo::espaciado() {
    return _espaciado;
}
// ------------------------------------------------------------------
void AnalizadorVideo::set_adaptativo(const bool &adaptativo) {
    _adaptativo = adaptativo;
}
// ------------------------------------------------------------------
void AnalizadorVideo::set_contraste(const double &contraste) {
    _contraste = contraste;
}
// ------------------------------------------------------------------
void AnalizadorVideo::set_num_frames(const int &num_frames) {
    _num_frames = num_frames;
}
// ------------------------------------------------------------------
void AnalizadorVideo::set_espaciado(const int &espaciado) {
    _espaciado = espaciado;
}
// ------------------------------------------------------------------
bool AnalizadorVideo::set_video(const std::string& path) {
    if (! Analizador::set_video(path, true))
        return false;

    string abs_path = build_abs_path(path);

    open_video_writer(_mascara_of, outfilename(abs_path, "_mask.avi"), false);
    if (!_mascara_of.isOpened()) {
        qCritical() << "set_video: Error creating mask output file";
        return false;
    }

    return true;
}
// ------------------------------------------------------------------
void AnalizadorVideo::mostrar_frame(const Mat &frame, const string &name) {
    int depth = frame.type() & CV_MAT_DEPTH_MASK;

    if (depth == CV_32F) {
        qDebug() << "AnalizadorVideo::mostrar_frame: 32F -> 8U";
        Mat clon;
        frame.convertTo(clon, CV_8U);
        Analizador::mostrar_frame(clon, name);
    } else
        Analizador::mostrar_frame(frame, name);
}
// ------------------------------------------------------------------
void AnalizadorVideo::analizar_video() {
    assert(_video.isOpened());

    int i = 0;
    if (! _adaptativo) {
        cargar_modelo();
        mostrar_frame(_modelo, "modelo-fondo");
    }

    Mat fg(_modelo.size(), CV_8UC3, Scalar(0, 0, 0));
    while(true) {
        _mostrar = i == NIGHT_SAMPLE_POS || i == DAY_SAMPLE_POS;

        if (_adaptativo && i % 100 == 0) {
            crear_modelo();
            mostrar_frame(_modelo, "Modelo-fondo-"+to_string(i));
        }

        _video >> _frame;
        if (i == 1051 || _frame.empty()) {
            qDebug() << "analizar_video: done";
            break;
        }

        if (_mostrar) qDebug() << "analizar_video: computing mask";
        calcular_mascara();
        if (_mostrar) qDebug() << "analizar_video: saving mask to output file";
        _mascara_of << _mascara;
        if (_mostrar) {
            qDebug() << "analizar_video: displaying mask";
            mostrar_frame(_mascara, "mascara-"+to_string(i));
            save_image(_mascara, "segm", "mascara"+to_string(i));
        }

        if (_mostrar) qDebug() << "analizar_video: applying mask";
        apply_mask(_frame, _mascara, fg);
        if (_mostrar) qDebug() << "analizar_video: saving foreground to output file";
        _salida << fg;

        if (_mostrar) {
            qDebug() << "analizar_video: displaying foreground";
            mostrar_frame(fg, "fg-"+to_string(i));
            save_image(fg, "segm", "fg"+to_string(i));
        }

        if (_mostrar) qDebug() << "analizar_video: frame " << i << " end";
        i++;
    }
    qDebug() << "analizar_video: total frames = " << i;
    _salida.release();
    _video.release();
}
// ------------------------------------------------------------------
void AnalizadorVideo::crear_imagen_media(string fp) {
    assert(_video.isOpened());

    qDebug() << "crear_imagen_media: creating background model";

    int num = 1;
    _video >> _modelo;
    _modelo.convertTo(_modelo, CV_32F);

    while (true) {
        _video >> _frame;

        if (_frame.empty()) break;

        _frame.convertTo(_frame, CV_32F);
        _modelo += _frame;
        num++;
        qDebug() << "crear_imagen_media: " << num << "frames";
    }
    _modelo /= num;

    imwrite(fp, _modelo);
    qDebug() << "crear_imagen_media: background model saved: "
             << QString::fromStdString(fp);

    mostrar_frame(_modelo, "imagen-media");
    _video.set(CAP_PROP_POS_MSEC, 0);
}
// ------------------------------------------------------------------
void AnalizadorVideo::crear_modelo() {
    double start = _video.get(CAP_PROP_POS_FRAMES);
    double next = start;

    _video >> _modelo;
    _modelo.convertTo(_modelo, CV_32FC3);

    for (int i = 1; i < _num_frames; i++) {
        _video.set(CAP_PROP_POS_FRAMES, next += _espaciado);

        Mat frame;
        _video >> frame;
        frame.convertTo(frame, CV_32FC3);
        _modelo += frame;
    }

    _modelo /= _num_frames;

    _video.set(CAP_PROP_POS_FRAMES, start);
}
// ------------------------------------------------------------------
void AnalizadorVideo::cargar_modelo() {
    stringstream ss;
    ss << Analizador::_wd << "imagen-media.png";
    string fp = ss.str();

    qDebug() << "cargar_modelo: attempting to load "
             << QString::fromStdString(fp);
    _modelo = imread(fp);

    if (_modelo.empty()) {
        qDebug() << "cargar_modelo: failed to load model";
        crear_imagen_media(fp);
    } else {
        qDebug() << "cargar_modelo: loading model success";
        qDebug() << "cargar_modelo: model mat type "
                 << QString::fromStdString(cv::typeToString(_modelo.type()));
    }
    _modelo *= _contraste;
    qDebug() << "cargar_modelo: contrast tweak alpha = " << _contraste;
}
// ------------------------------------------------------------------
void AnalizadorVideo::calcular_mascara() {
    Mat diferencia(_modelo.size(), CV_8U);

    _frame *= _contraste;

    absdiff(_frame, _modelo, diferencia);
    cvtColor(diferencia, diferencia, COLOR_RGB2GRAY);

    double thr = threshold(diferencia, _mascara, 0, 255.0, cv::THRESH_OTSU);
    if (_mostrar)
        qDebug() << "calcular_diferencia: umbral = " << thr;
}
