#include "Segmentator.h"
#include "stdio.h"
#include <windows.h>
#include <string>
#include <fstream>

#include <QDebug>

#include <Utils.h>

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
void Segmentator::set_wd(const string &path) {
    _wd = path;
}
// ------------------------------------------------------------------
bool Segmentator::set_video(const std::string &path) {
    // Set working directory
    _wd = common::get_wd_from(path);
    if (_wd.empty()) {
        qCritical() << "set_video: Path must be absolute. Path:" << QString::fromStdString(path);
        return false;
    }
    qDebug() << "set_video: " << QString::fromStdString(path);

    // Set video input
    if (_video.isOpened())
        _video.release();

    _video.open(path);
    if ( !_video.isOpened()) {
        qCritical() << "set_video: Error opening provided video file";
        return false;
    }

    // Set video output(s)
    return open_video_outputs(path);
}
// ------------------------------------------------------------------
bool Segmentator::open_video_outputs(const std::string &path) {
    open_video_writer(_salida, common::outfilename(path, "_processed.avi"), false);
    if ( !_salida.isOpened()) {
        qCritical() << "set_video: Error creating output file _salida";
        return false;
    }
    return true;
}
// ------------------------------------------------------------------
bool Segmentator::open_video_writer(cv::VideoWriter &writer, const std::string &file, const bool &isColor) {
    if (writer.isOpened())
        writer.release();

    writer.open(file,
                static_cast<int>(_video.get(CAP_PROP_FOURCC)),
                _video.get(CAP_PROP_FPS),
                Size(static_cast<int>(_video.get(CAP_PROP_FRAME_WIDTH)),
                     static_cast<int>(_video.get(CAP_PROP_FRAME_HEIGHT) - BAR_HEIGHT)),
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

        _salida << _output;
    }
    qDebug() << "analizar_video: END total =" << _pos - 1 << "frames";

    _video.release();
    _salida.release();
}
// -------------------------------------------------------------------
void Segmentator::process_frame(const Mat &frame, Mat &output) {
    _frame = frame;
    process_frame();
    output = _output;
    _pos++;
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
    common::show_image(frame, name);
}
// -------------------------------------------------------------------
void Segmentator::open(const Mat &src, Mat &dst) {
    Mat kernel = getStructuringElement(MORPH_CROSS, Size(3, 3));
    morphologyEx(src, dst, MORPH_OPEN, kernel);
}
// -------------------------------------------------------------------
void Segmentator::invert(const Mat &src, Mat &dst) {
    dst = 255 - src;
}
