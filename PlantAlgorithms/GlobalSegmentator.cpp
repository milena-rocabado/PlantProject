#include "GlobalSegmentator.h"

#include "common.h"

#include <stdio.h>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

static uint pos;
static bool dump;

void global_process(const Mat &src, Mat &dst) {

    // Recortar
    crop_roi(src, dst);
    if (dump)
        save("Global/" + to_string(pos) + "_cropped", dst);

    // A escala de grises
    cvtColor(dst, dst, COLOR_RGB2GRAY);
    if (dump)
        save("Global/" + to_string(pos) + "_gs", dst);

    // Aumentar contraste
    dst *= GLOBAL_SEGM_ALPHA;
    if (dump)
        save("Global/" + to_string(pos) + "_a", dst);

    // Umbralizar
    double thr = 132.526;
    threshold(dst, dst, thr, 255, THRESH_BINARY);
    if (dump)
        save("Global/" + to_string(pos) + "_thr", dst);
}

void run_global_segmentator() {
    string wd = "C:/Users/milena/git/PlantProject/Media/";

    VideoCapture video("C:/Users/milena/git/PlantProject/Media/climbing_bean_project3_leaf_folding.AVI");

    int  out_width = static_cast<int>(video.get(CAP_PROP_FRAME_WIDTH) / 3.0); // / 3;
    int out_height = static_cast<int>(video.get(CAP_PROP_FRAME_HEIGHT)) - BAR_HEIGHT;
    VideoWriter salida("C:/Users/milena/git/PlantProject/Media"
                      "/global_dump_out.AVI", static_cast<int>(video.get(CAP_PROP_FOURCC)),
                       video.get(CAP_PROP_FPS),
                       Size(out_width, out_height), false);
    Mat frame;
    Mat output;

    for (pos = 0u; ; pos++, dump = (pos == NIGHT_SAMPLE || pos == DAY_SAMPLE) ) {
        video >> frame;

        if (pos == 1100 /*|| frame.empty()*/) break;
        if (pos % 150 == 0)
            cout << "run_global_segmentator: analizados " + to_string(pos/30) + " segundos" << endl;

        global_process(frame, output);

        salida << output;
    }
}
