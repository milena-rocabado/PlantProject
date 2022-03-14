#include "Segmentator.h"
#include "stdio.h"
#include <windows.h>
#include <string>
#include <fstream>

#include <QDebug>

using namespace std;
using namespace cv;

// -------------------------------------------------------------------
Segmentator::~Segmentator() {
    _frame.release();
    if (_video.isOpened())
        _video.release();
    if (_salida.isOpened())
        _salida.release();
}
// ------------------------------------------------------------------
bool Segmentator::set_video(const string &path) {
    return Segmentator::set_video(path, false, true);
}
// ------------------------------------------------------------------
bool Segmentator::set_video(const std::string &path, const bool &isColor, const bool &isCropped) {
    if (! get_wd_from(path)) {
        qCritical() << "set_video: Path must be absolute";
        return false;
    }
    qDebug() << "set_video: " << QString::fromStdString(path);

    if (_video.isOpened())
        _video.release();

    _video.open(path);
    if ( !_video.isOpened()) {
        qCritical() << "set_video: Error opening provided video file";
        return false;
    }

    open_video_writer(_salida, outfilename(path, "_processed.avi"), isColor, isCropped);
    if ( !_salida.isOpened()) {
        qCritical() << "set_video: Error creating output file _salida";
        return false;
    } qDebug() << "set_video: ouput video isColor = " << isColor;

    return true;
}
// ------------------------------------------------------------------
bool Segmentator::open_video_writer(cv::VideoWriter &writer, const std::string &file, const bool &isColor, const bool &isCropped) {
    if (writer.isOpened())
        writer.release();

    int crop = isCropped ? 20 : 0;
    writer.open(file,
                static_cast<int>(_video.get(CAP_PROP_FOURCC)),
                _video.get(CAP_PROP_FPS),
                Size(static_cast<int>(_video.get(CAP_PROP_FRAME_WIDTH)),
                     static_cast<int>(_video.get(CAP_PROP_FRAME_HEIGHT) - crop)),
                isColor);

    return writer.isOpened();
}
// ------------------------------------------------------------------
void Segmentator::process_timed() {
    start_timer();
    qDebug() << "analizar_timed: timer started";
    process_video();
    stop_timer();
    qDebug() << "analizar_timed: Time =" << time();
}
// ------------------------------------------------------------------
void Segmentator::process_video() {
    assert(_video.isOpened());
    assert(_salida.isOpened());

    set_up();

    qDebug() << "analizar_video: START";
    for (_pos = 0; ; _pos++) {
        _video >> _frame;

        if (_frame.empty()) break;

        _ejemplo = _pos == NIGHT_SAMPLE_POS || _pos == DAY_SAMPLE_POS || _pos == 0;

        if (_pos % 150 == 0)
            qDebug() << "analizar_video:" << _pos / 30 << "segundos analizados";

        process_frame();

        _salida << _resultado;
    }
    qDebug() << "analizar_video: END total =" << _pos - 1 << "frames";

    _video.release();
    _salida.release();
}
// ------------------------------------------------------------------
void Segmentator::show_video() {
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
void Segmentator::show_frame(const Mat &frame, const string &name) {
    namedWindow(name, WINDOW_NORMAL);
    moveWindow(name, 0, 0);
    resizeWindow(name, frame.size().width, frame.size().height);
    imshow(name, frame);
    waitKey(1);
}
// ------------------------------------------------------------------
string Segmentator::outfilename(const string &filename, const string &suffix) {
    size_t lastdot = filename.find_last_of(".");
    if (lastdot == string::npos) return filename + suffix;
    return filename.substr(0, lastdot) + suffix;
}
// ------------------------------------------------------------------
bool Segmentator::get_wd_from(const string &path) {
    size_t last_slash = path.find_last_of("/");
    if (last_slash != string::npos) {
        _wd = path.substr(0, last_slash + 1);
        return true;
    } else return false;
}
// ------------------------------------------------------------------
string Segmentator::build_abs_path(const string &path) {
    assert(!_wd.empty());
    stringstream ss;
    ss << _wd << path;
    return ss.str();
}
// ------------------------------------------------------------------
void Segmentator::save_image(const Mat &im, const string &dir, const string &filename) {
    string fp = build_abs_path("Ejemplos/"  + dir + "/" + filename + ".png");
    imwrite(fp, im);
}
// ------------------------------------------------------------------
void Segmentator::save_image(const Mat &im, const string &filename) {
    string fp = build_abs_path("Ejemplos/" + filename + ".png");
    imwrite(fp, im);
}
// ------------------------------------------------------------------
void Segmentator::apply_mask(const Mat &img, const Mat &mask, Mat &dst) {
    dst.release();
    img.copyTo(dst, mask);
}
// ------------------------------------------------------------------
void Segmentator::crop_time_bar(cv::Mat &img) {
    crop_time_bar(img, img);
}
// ------------------------------------------------------------------
void Segmentator::crop_time_bar(const Mat &src, Mat &dst) {
    Rect roi(0, 0, src.size().width, src.size().height - BAR_HEIGHT);
    dst = src(roi);
}
// -------------------------------------------------------------------
void Segmentator::abrir(Mat &img) {
    Mat kernel = getStructuringElement(MORPH_RECT, Size(3, 3));
    morphologyEx(img, img, MORPH_OPEN, kernel);
}
// -------------------------------------------------------------------
void Segmentator::invertir(Mat &img) {
    img = 255 - img;
}
