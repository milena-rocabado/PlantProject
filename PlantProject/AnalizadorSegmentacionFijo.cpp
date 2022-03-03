#include "AnalizadorSegmentacionFijo.h"
#include <QtDebug>

using namespace std;
using namespace cv;

AnalizadorSegmentacionFijo::~AnalizadorSegmentacionFijo() {
    _modelo.release();
    _mascara.release();
}
// ------------------------------------------------------------------
bool AnalizadorSegmentacionFijo::adaptativo() {
    return _adaptativo;
}
// ------------------------------------------------------------------
double AnalizadorSegmentacionFijo::contraste() {
    return _contraste;
}
// ------------------------------------------------------------------
int AnalizadorSegmentacionFijo::num_frames() {
    return _num_frames;
}
// ------------------------------------------------------------------
int AnalizadorSegmentacionFijo::espaciado() {
    return _espaciado;
}
// ------------------------------------------------------------------
int AnalizadorSegmentacionFijo::blur() {
    return _blur;
}
// ------------------------------------------------------------------
void AnalizadorSegmentacionFijo::set_adaptativo(const bool &adaptativo) {
    _adaptativo = adaptativo;
}
// ------------------------------------------------------------------
void AnalizadorSegmentacionFijo::set_contraste(const double &contraste) {
    _contraste = contraste;
    _apply_contrast = trunc(_contraste * 100.) == 100; // _contraste == 1.00
}
// ------------------------------------------------------------------
void AnalizadorSegmentacionFijo::set_num_frames(const int &num_frames) {
    _num_frames = num_frames;
}
// ------------------------------------------------------------------
void AnalizadorSegmentacionFijo::set_espaciado(const int &espaciado) {
    _espaciado = espaciado;
}
// ------------------------------------------------------------------
void AnalizadorSegmentacionFijo::set_blur(const int &blur) {
    _blur = blur;
    _apply_blur = _blur != 0;
}
// ------------------------------------------------------------------
bool AnalizadorSegmentacionFijo::set_video(const std::string& path) {
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
void AnalizadorSegmentacionFijo::mostrar_frame(const Mat &frame, const string &name) {
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
void AnalizadorSegmentacionFijo::analizar_2_frames() {
    _apply_blur = false;
    _apply_contrast = false;
    cargar_modelo();

    _video.set(CAP_PROP_POS_FRAMES, NIGHT_SAMPLE_POS);
    _video >> _frame;
    _i = NIGHT_SAMPLE_POS;
    qDebug() << "analizar_2_frames: loaded night frame";
    iterar_umbrales();

    _video.set(CAP_PROP_POS_FRAMES, DAY_SAMPLE_POS);
    _video >> _frame;
    _i = DAY_SAMPLE_POS;
    qDebug() << "analizar_2_frames: loaded day frame";
    iterar_umbrales();

    qDebug() << "analizar_2_frames: done.";
}
// ------------------------------------------------------------------
void AnalizadorSegmentacionFijo::analizar_video() {
    assert(_video.isOpened());

    _i = 0;

    if (! _adaptativo) {
        cargar_modelo();
        mostrar_frame(_modelo, "modelo-fondo");
    }

    Mat fg(_modelo.size(), CV_8UC3, Scalar(0, 0, 0));
    while(true) {
        _ejemplo = _i == NIGHT_SAMPLE_POS || _i == DAY_SAMPLE_POS;

        if (_adaptativo && _i % 100 == 0) {
            crear_modelo();
            mostrar_frame(_modelo, "Modelo-fondo-"+to_string(_i));
        }

        _video >> _frame;
        if (_i == 1051 || _frame.empty()) {
            qDebug() << "analizar_video: done";
            break;
        }

        if (_ejemplo) qDebug() << "analizar_video: computing mask";
        calcular_mascara();
        _mascara_of << _mascara;
        if (_ejemplo) {
            qDebug() << "analizar_video: displaying mask";
            mostrar_frame(_mascara, "mascara-"+to_string(_i));
            save_image(_mascara, "segm", "mascara"+to_string(_i));
        }

        if (_ejemplo) qDebug() << "analizar_video: applying mask";
        apply_mask(_frame, _mascara, fg);
        _salida << fg;

        if (_ejemplo) {
            qDebug() << "analizar_video: displaying foreground";
            mostrar_frame(fg, "fg-"+to_string(_i));
            save_image(fg, "segm", "fg"+to_string(_i));
        }

        if (_ejemplo) qDebug() << "analizar_video: frame " << _i << " end";
        _i++;
    }
    qDebug() << "analizar_video: total frames = " << _i;
    _salida.release();
    _video.release();
}
// ------------------------------------------------------------------
void AnalizadorSegmentacionFijo::crear_imagen_media(string fp) {
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
void AnalizadorSegmentacionFijo::crear_modelo() {
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
void AnalizadorSegmentacionFijo::cargar_modelo() {
    stringstream ss;
    ss << Analizador::_wd << "imagen-media.png";
    string fp = ss.str();

    qDebug() << "cargar_modelo: attempting to load "
             << QString::fromStdString(fp);
    _modelo = imread(fp);

    if (_modelo.empty()) {
        qDebug() << "cargar_modelo: failed to load model";
        crear_imagen_media(fp);
    } else qDebug() << "cargar_modelo: loading model success";

    cvtColor(_modelo, _modelo, COLOR_BGR2GRAY);
    qDebug() << "cargar_modelo: modelo converted to grayscale";

    crop_time_bar(_modelo);
    qDebug() << "cargar_modelo: time bar cropped";

    if (_apply_contrast) {
        _modelo *= _contraste;
        qDebug() << "cargar_modelo: contrast tweak alpha = " << _contraste;
    }

    if (_apply_blur) {
        GaussianBlur(_modelo, _modelo, Size(_blur, _blur), 0);
        qDebug() << "cargar_modelo: blur = " << _blur;
    }
}
// ------------------------------------------------------------------
void AnalizadorSegmentacionFijo::calcular_mascara() {
    Mat diferencia(_modelo.size(), CV_8U);

    crop_time_bar(_frame);
    cvtColor(_frame, _frame, COLOR_BGR2GRAY);

    if (_apply_contrast) _frame *= _contraste;
    absdiff(_frame, _modelo, diferencia);

    threshold(diferencia, _mascara, 0, 255.0, cv::THRESH_OTSU);
}
// ------------------------------------------------------------------
void AnalizadorSegmentacionFijo::iterar_contrastes() {
    Mat dif(_modelo.size(), CV_8U);
    Mat con_dif;
    bool show = true;

    crop_time_bar(_frame);
    qDebug() << "iterar_contrastes: frame cropped";
    cvtColor(_frame, _frame, COLOR_BGR2GRAY);
    qDebug() << "iterar_contrastes: frame to grayscale";
    dif = _modelo - _frame;
    qDebug() << "iterar_contrastes: dif computed";
    save_image(dif, "segm/contraste", to_string(_i) + "-dif-a" + to_string(1.0));

    double thr = threshold(dif, _mascara, 0, 255.0, cv::THRESH_OTSU);
    qDebug() << "iterar_contrastes: umbral =" << thr << "a =" << 1.0;
    save_image(_mascara, "segm/contraste", to_string(_i) + "-mask-a" + to_string(1.0));

    for (double i = 1.5; i <= 4.0; i += 0.5) {
        con_dif = dif * i;
        if (show) mostrar_frame(con_dif, to_string(_i) + "-dif-a" + to_string(i));
        show = trunc(i * 100) == 350;
        save_image(con_dif, "segm/contraste", to_string(_i) + "-dif-a" + to_string(i));

        //double thr = threshold(dif, _mascara, 0, 255.0, cv::THRESH_OTSU);
        double thr = 80.0;
        threshold(con_dif, _mascara, thr, 255, THRESH_BINARY);
        qDebug() << "iterar_contrastes: umbral =" << thr << "a =" << i;
        save_image(_mascara, "segm/contraste", to_string(_i) + "-mask-a" + to_string(i));
    }
}
// ------------------------------------------------------------------
void AnalizadorSegmentacionFijo::iterar_blur() {
    Mat dif(_modelo.size(), CV_8U);
    Mat blurred_dif;

    crop_time_bar(_frame);
    qDebug() << "iterar_blur: frame cropped";
    cvtColor(_frame, _frame, COLOR_BGR2GRAY);
    qDebug() << "iterar_blur: frame to grayscale";
    dif = _modelo - _frame;
    qDebug() << "iterar_blur: dif computed";
    save_image(dif, "segm/blur", to_string(_i) + "-dif");

    double thr = threshold(dif, _mascara, 0, 255.0, cv::THRESH_OTSU);
    qDebug() << "iterar_blur: umbral = " << thr << "kernel = " << 0;
    save_image(_mascara, "segm/blur", to_string(_i) + "-noblur");

    for (int i = 3; i <= 15; i+=4) {
        GaussianBlur(dif, blurred_dif, Size(i, i), 0);
        double thr = threshold(blurred_dif, _mascara, 0, 255.0, cv::THRESH_OTSU);

        qDebug() << "iterar_blur: umbral = " << thr << "kernel = " << i;
        save_image(_mascara, "segm/blur", to_string(_i) + "-blur" + to_string(i));
    }
}
// ------------------------------------------------------------------
void AnalizadorSegmentacionFijo::iterar_umbrales() {
    Mat dif(_modelo.size(), CV_8U);

    crop_time_bar(_frame);
    qDebug() << "iterar_umbrales: frame cropped";
    cvtColor(_frame, _frame, COLOR_BGR2GRAY);
    qDebug() << "iterar_umbrales: frame to grayscale";
    dif = _modelo - _frame;
    qDebug() << "iterar_umbrales: dif computed";
    save_image(dif, "segm/thr", to_string(_i) + "-dif");

    dif *= 4.0;
    for (int i = 60; i <= 120; i+=10) {
        double thr = i;
        threshold(dif, _mascara, thr, 255, THRESH_BINARY);
        qDebug() << "iterar_blur: umbral =" << i;
        save_image(_mascara, "segm/thr", to_string(_i) + "-thr" + to_string(i));
    }
}

// ------------------------------------------------------------------
void AnalizadorSegmentacionFijo::procesar() {
    Mat dif(_modelo.size(), CV_8U);

    crop_time_bar(_frame);
    qDebug() << "procesar: frame cropped";
    cvtColor(_frame, _frame, COLOR_BGR2GRAY);
    qDebug() << "procesar: frame to grayscale";
    dif = _modelo - _frame;
    qDebug() << "procesar: dif computed";
    mostrar_frame(dif, to_string(_i) + "-dif");
    save_image(dif, "segm", to_string(_i) + "-dif");

    double umbral = 20.0;
    threshold(dif, _mascara, umbral, 255, THRESH_BINARY);
    qDebug() << "procesar: umbral = " << umbral;
    mostrar_frame(_mascara, to_string(_i) + "-mask");
    save_image(_mascara, "segm", to_string(_i) + "-mask");
}
