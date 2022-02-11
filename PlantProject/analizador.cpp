#include "analizador.h"
#include "stdio.h"
#include <windows.h>
#include <string>
#include <fstream>

using namespace std;
using namespace cv;

bool Analizador::set_video(string path) {
    cout << path << endl;
    if (_video.isOpened())
        _video.release();

    _video.open(path);

    if (_salida.isOpened())
        _salida.release();

    _salida.open(outfilename(path),
                static_cast<int>(_video.get(CAP_PROP_FOURCC)),
                _video.get(CAP_PROP_FPS),
                Size(static_cast<int>(_video.get(CAP_PROP_FRAME_WIDTH)),
                     static_cast<int>(_video.get(CAP_PROP_FRAME_HEIGHT))), false);

    return _video.isOpened() && _salida.isOpened();
}
// ------------------------------------------------------------------
string Analizador::outfilename(const string& filename) {
    size_t lastdot = filename.find_last_of(".");
    if (lastdot == string::npos) return filename;
    return filename.substr(0, lastdot).append("_thr.avi");
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
