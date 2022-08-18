#include "Analyzer.h"

#include <Utils.h>

#include <Segmentator.h>
#include <GlobalSegmentator.h>
#include <StaticModelSegmentator.h>

#include <Calculator.h>

#include <QDebug>

using namespace std;
using namespace cv;

constexpr int Calculator::OUTPUT_HEIGHT_ADJ;

Analyzer::Analyzer(const SegmType &segmType) {
    switch (segmType) {
    case Global:
        _segm = new GlobalSegmentator();
        break;
    case StaticModel:
        _segm = new StaticModelSegmentator();
    }
    _calc = new Calculator();
}
// ------------------------------------------------------------------
bool Analyzer::set_video(const std::string &path) {
    _wd = common::get_wd_from(path);
    if (_wd.empty()) {
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

    if (! open_video_writer(_mask, common::outfilename(path, "_processed.avi"), false,
                            BAR_HEIGHT)) {
        qCritical() << "set_video: Error creating output file _salida";
        return false;
    } else qDebug() << "set_video: success opening _salida";

    if (! open_video_writer(_sth, common::outfilename(path, "_cropped.avi"), /*false*/ true,
                            BAR_HEIGHT)) {
        qCritical() << "set_video: Error creating output file _sth";
        return false;
    } else qDebug() << "set_video: success opening _sth";

    qDebug() << "set_video: completed correctly";
    return true;
}
// ------------------------------------------------------------------
bool Analyzer::open_video_writer(cv::VideoWriter &writer, const std::string &file,
                                 const bool &isColor, const int &height_adj) {

    qDebug() << "open_video_writer: at" << QString::fromStdString(file)
             << "isColor:" << isColor;

    writer.open(file,
                static_cast<int>(_video.get(CAP_PROP_FOURCC)),
                _video.get(CAP_PROP_FPS),
                Size(static_cast<int>(_video.get(CAP_PROP_FRAME_WIDTH)),
                     static_cast<int>(_video.get(CAP_PROP_FRAME_HEIGHT)) - height_adj),
                isColor);

    qDebug() << "open_video_writer: frame size =" << _video.get(CAP_PROP_FRAME_WIDTH)
             << "x" << (_video.get(CAP_PROP_FRAME_HEIGHT) - height_adj);

    return writer.isOpened();
}
// ------------------------------------------------------------------
void Analyzer::process_video() {
    assert(_video.isOpened());
    assert(_mask.isOpened());
    _segm->set_up();

    Mat frame, mask, sth;
    for (_pos = 0; ; _pos++) {
        _video >> frame;
        if (_pos == Segmentator::DAY_SAMPLE_POS+1 || frame.empty()) break;

        _segm->process_frame(frame, mask);
        _mask << mask;

        _calc->process_frame(mask, sth);
        _sth << sth;

        if (_pos % 150 == 0) {
            qDebug() << "process_video:" << _pos/30 << "seconds";
        }
    }
    qDebug() << "process_video: Total" << _pos << "frames";

    _video.release();
    _mask.release();
}
