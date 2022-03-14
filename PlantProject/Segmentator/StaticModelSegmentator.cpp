#include "StaticModelSegmentator.h"
#include <QtDebug>

using namespace std;
using namespace cv;

// ------------------------------------------------------------------
StaticModelSegmentator::~StaticModelSegmentator() {
    _model.release();
}
// ------------------------------------------------------------------
void StaticModelSegmentator::show_frame(const Mat &frame, const string &name) {
    int depth = frame.type() & CV_MAT_DEPTH_MASK;

    if (depth == CV_32F) {
        qDebug() << "AnalizadorVideo::mostrar_frame: 32F -> 8U";
        Mat clon;
        frame.convertTo(clon, CV_8U);
        Segmentator::show_frame(clon, name);
    } else
        Segmentator::show_frame(frame, name);
}
// ------------------------------------------------------------------
void StaticModelSegmentator::process_2_frames() {
    load_model();

    _video.set(CAP_PROP_POS_FRAMES, NIGHT_SAMPLE_POS);
    _video >> _frame;
    _pos = NIGHT_SAMPLE_POS;
    qDebug() << "analizar_2_frames: loaded night frame";
    process_frame();

    _video.set(CAP_PROP_POS_FRAMES, DAY_SAMPLE_POS);
    _video >> _frame;
    _pos = DAY_SAMPLE_POS;
    qDebug() << "analizar_2_frames: loaded day frame";
    process_frame();

    qDebug() << "analizar_2_frames: done.";
}
// ------------------------------------------------------------------
void StaticModelSegmentator::load_model() {
    stringstream ss;
    ss << Segmentator::_wd << (_conf.model == Modelo1
                              ? "modelo_fondo.png"
                              : "modelo_noche.png");
    string fp = ss.str();

    qDebug() << "cargar_modelo: attempting to load "
             << QString::fromStdString(fp);
    _model = imread(fp);

    if (_model.empty()) {
        qDebug() << "cargar_modelo: failed to load model";
        create_mean_image(fp);
    } else
        qDebug() << "cargar_modelo: loading model success";

    cvtColor(_model, _model, COLOR_BGR2GRAY);
    qDebug() << "cargar_modelo: model converted to grayscale";

    crop_time_bar(_model);
    qDebug() << "cargar_modelo: time bar cropped";
}
// ------------------------------------------------------------------
void StaticModelSegmentator::create_mean_image(string fp) {
    assert(_video.isOpened());

    qDebug() << "crear_imagen_media: creating background model";

    int num = 1;
    _video >> _model;
    _model.convertTo(_model, CV_32F);

    while (true) {
        _video >> _frame;

        if (_frame.empty()) break;

        _frame.convertTo(_frame, CV_32F);
        _model += _frame;
        num++;
        qDebug() << "crear_imagen_media: " << num << "frames";
    }
    _model /= num;

    imwrite(fp, _model);
    qDebug() << "crear_imagen_media: background model saved: "
             << QString::fromStdString(fp);

    _video.set(CAP_PROP_POS_MSEC, 0);
}
// ------------------------------------------------------------------
void StaticModelSegmentator::process_frame() {
    Mat diferencia(_model.size(), CV_8U);

    crop_time_bar(_frame);
    cvtColor(_frame, _frame, COLOR_BGR2GRAY);

    diferencia = _model - _frame;
    if (_apply_contrast) {
        diferencia *= _conf.alpha;
    }

    threshold(diferencia, _resultado, _conf.thresh, 255.0, THRESH_BINARY);
}
// ------------------------------------------------------------------
void StaticModelSegmentator::iterate_alpha() {
    Mat dif(_model.size(), CV_8U);
    Mat con_dif;
    bool show = true;

    crop_time_bar(_frame);
    qDebug() << "iterar_contrastes: frame cropped";
    cvtColor(_frame, _frame, COLOR_BGR2GRAY);
    qDebug() << "iterar_contrastes: frame to grayscale";
    dif = _model - _frame;
    qDebug() << "iterar_contrastes: dif computed";
    save_image(dif, "segm/contraste", to_string(_pos) + "-dif-a" + to_string(1.0));

    double thr = threshold(dif, _resultado, 0, 255.0, cv::THRESH_OTSU);
    qDebug() << "iterar_contrastes: umbral =" << thr << "a =" << 1.0;
    save_image(_resultado, "segm/contraste", to_string(_pos) + "-mask-a" + to_string(1.0));

    for (double i = 1.5; i <= 4.0; i += 0.5) {
        con_dif = dif * i;
        if (show) show_frame(con_dif, to_string(_pos) + "-dif-a" + to_string(i));
        show = trunc(i * 100) == 350;
        save_image(con_dif, "segm/contraste", to_string(_pos) + "-dif-a" + to_string(i));

        double thr = 80.0;
        threshold(con_dif, _resultado, thr, 255, THRESH_BINARY);
        qDebug() << "iterar_contrastes: umbral =" << thr << "a =" << i;
        save_image(_resultado, "segm/contraste", to_string(_pos) + "-mask-a" + to_string(i));
    }
}
// ------------------------------------------------------------------
void StaticModelSegmentator::iterate_blur() {
    Mat dif(_model.size(), CV_8U);
    Mat blurred_dif;

    crop_time_bar(_frame);
    qDebug() << "iterar_blur: frame cropped";
    cvtColor(_frame, _frame, COLOR_BGR2GRAY);
    qDebug() << "iterar_blur: frame to grayscale";
    dif = _model - _frame;
    qDebug() << "iterar_blur: dif computed";
    save_image(dif, "segm/blur", to_string(_pos) + "-dif");

    double thr = threshold(dif, _resultado, 0, 255.0, cv::THRESH_OTSU);
    qDebug() << "iterar_blur: umbral = " << thr << "kernel = " << 0;
    save_image(_resultado, "segm/blur", to_string(_pos) + "-noblur");

    for (int i = 3; i <= 15; i+=4) {
        GaussianBlur(dif, blurred_dif, Size(i, i), 0);
        double thr = threshold(blurred_dif, _resultado, 0, 255.0, cv::THRESH_OTSU);

        qDebug() << "iterar_blur: umbral = " << thr << "kernel = " << i;
        save_image(_resultado, "segm/blur", to_string(_pos) + "-blur" + to_string(i));
    }
}
// ------------------------------------------------------------------
void StaticModelSegmentator::iterate_thresh() {
    Mat dif(_model.size(), CV_8U);

    crop_time_bar(_frame);
    qDebug() << "iterar_umbrales: frame cropped";
    cvtColor(_frame, _frame, COLOR_BGR2GRAY);
    qDebug() << "iterar_umbrales: frame to grayscale";
    dif = _model - _frame;
    qDebug() << "iterar_umbrales: dif computed";
    save_image(dif, "segm/thr", to_string(_pos) + "-dif");

    dif *= _conf.alpha;
    for (int i = 60; i <= 120; i+=10) {
        double thr = i;
        threshold(dif, _resultado, thr, 255, THRESH_BINARY);
        qDebug() << "iterar_blur: umbral =" << i;
        save_image(_resultado, "segm/thr", to_string(_pos) + "-thr" + to_string(i));
    }
}
