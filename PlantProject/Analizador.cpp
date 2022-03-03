#include "Analizador.h"
#include "stdio.h"
#include <windows.h>
#include <string>
#include <fstream>

#include <QDebug>

using namespace std;
using namespace cv;

VideoCapture Analizador::_video;
VideoWriter Analizador::_salida;
Mat Analizador::_frame;
string Analizador::_wd;

// -------------------------------------------------------------------
Analizador::~Analizador() {
    _frame.release();
    if (_video.isOpened())
        _video.release();
    if (_salida.isOpened())
        _salida.release();
}
// -------------------------------------------------------------------
string Analizador::wd() {
    return _wd;
}
// -------------------------------------------------------------------
void Analizador::set_wd(const string &wd) {
    _wd = wd;
}
// ------------------------------------------------------------------
bool Analizador::set_video(const string &path) {
    return Analizador::set_video(path, false);
}
// ------------------------------------------------------------------
bool Analizador::set_video(const string &path, const bool &isColor) {
    string abs_path = build_abs_path(path);
    qDebug() << "set_video: " << QString::fromStdString(abs_path);

    if (_video.isOpened())
        _video.release();

    _video.open(abs_path);
    if ( !_video.isOpened()) {
        qCritical() << "set_video: Error opening provided video file";
        return false;
    }

    open_video_writer(_salida, outfilename(abs_path, "_processed.avi"), isColor);
    if ( !_salida.isOpened()) {
        qCritical() << "set_video: Error creating output file _salida";
        return false;
    } qDebug() << "set_video: ouput video isColor = " << isColor;

    return true;
}
// ------------------------------------------------------------------
bool Analizador::open_video_writer(cv::VideoWriter &writer, const std::string &file, const bool &isColor) {
    if (writer.isOpened())
        writer.release();
    writer.open(file,
                static_cast<int>(_video.get(CAP_PROP_FOURCC)),
                _video.get(CAP_PROP_FPS),
                Size(static_cast<int>(_video.get(CAP_PROP_FRAME_WIDTH)),
                     static_cast<int>(_video.get(CAP_PROP_FRAME_HEIGHT))),
                isColor);

    return writer.isOpened();
}
// ------------------------------------------------------------------
void Analizador::show_video() {
    assert(_video.isOpened());
    Mat frame;
    unsigned long ms = static_cast<unsigned long>(1000/_video.get(CAP_PROP_FPS));

    if (! _video.isOpened()) return;

    namedWindow("_video", WINDOW_NORMAL);
    moveWindow("_video", 10, 10);

    while (true) {
        _video >> frame;

        if (frame.empty()) {
            break;
        }

        imshow("_video", frame);
        Sleep(ms);
    }
}
// -------------------------------------------------------------------
void Analizador::mostrar_frame(const Mat &frame, const string &name) {
    qDebug() << "Analizador::mostrar_frame: " << QString::fromStdString(name);
    namedWindow(name, WINDOW_NORMAL);
    moveWindow(name, 0, 0);
    resizeWindow(name, frame.size().width, frame.size().height);
    imshow(name, frame);
    waitKey(1);
}

// ------------------------------------------------------------------
string Analizador::outfilename(const string &filename, const string &suffix) {
    size_t lastdot = filename.find_last_of(".");
    if (lastdot == string::npos) return filename;
    return filename.substr(0, lastdot).append(suffix);
}
// ------------------------------------------------------------------
string Analizador::build_abs_path(const string &path) {
    assert(!_wd.empty());
    stringstream ss;
    ss << _wd << path;
    return ss.str();
}
// ------------------------------------------------------------------
void Analizador::save_image(const Mat &im, const string &dir, const string &filename) {
    string fp = build_abs_path("Ejemplos/"  + dir + "/" + filename + ".png");
    imwrite(fp, im);
}
// ------------------------------------------------------------------
void Analizador::apply_mask(const Mat &img, const Mat &mask, Mat &dst) {
    dst.release();
    img.copyTo(dst, mask);
}
// ------------------------------------------------------------------
void Analizador::crop_time_bar(cv::Mat &img) {
    Rect roi(0, 0, img.size().width, img.size().height - 20);
    img = img(roi);
}
// -------------------------------------------------------------------
void Analizador::abrir(Mat &img) {
    Mat kernel = getStructuringElement(MORPH_RECT, Size(3, 3));
    morphologyEx(img, img, MORPH_OPEN, kernel);
}
