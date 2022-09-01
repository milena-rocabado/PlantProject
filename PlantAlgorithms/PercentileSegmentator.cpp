#include "PercentileSegmentator.h"

#include "common.h"

#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <fstream>

#include "globalsegmentator.h"

using namespace std;
using namespace cv;

static uint pos;
static bool dump;

static ofstream ofile;

void preprocess(const Mat &src, Mat &dst) {

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

void calculate_percentile(Histogram &hist, int &num_samples, float &sum, const Mat &frame) {
    Mat histogram;
    hist(frame, histogram);

    float num = histogram.at<float>(0) / (frame.cols * frame.rows);

    num_samples++;
    sum += num;

    ofile << to_string(num) << endl;
}

void determine_percentile() {
    string wd = "C:/Users/milena/git/PlantProject/Media/";

    VideoCapture video("C:/Users/milena/git/PlantProject/Media/climbing_bean_project3_leaf_folding.AVI");

    int  out_width = static_cast<int>(video.get(CAP_PROP_FRAME_WIDTH)) / 3;
    int out_height = static_cast<int>(video.get(CAP_PROP_FRAME_HEIGHT)) - BAR_HEIGHT;
    VideoWriter salida("C:/Users/milena/git/PlantProject/Media"
                      "/global_dump_out.AVI", static_cast<int>(video.get(CAP_PROP_FOURCC)),
                       video.get(CAP_PROP_FPS),
                       Size(out_width, out_height), false);

    ofile.open(WD + string("percentiles.txt"));
    int num_samples { 0 };
    float sum { 0.0f };

    Histogram hist;
    hist.set_hist_size(2);

    Mat frame;
    Mat output;

    for (pos = 0u; ; pos++, dump = (pos == NIGHT_SAMPLE || pos == DAY_SAMPLE) ) {
        video >> frame;

        if (/*pos == 1100 || */frame.empty()) break;
        if (pos % 150 == 0)
            cout << "run_percentile_segmentator: analizados " + to_string(pos/30) + " segundos" << endl;

        preprocess(frame, output);
        calculate_percentile(hist, num_samples, sum, output);

        salida << output;
    }

    cout << "run_percentile_segmentator: percentil = " << sum /num_samples << endl;
}

void percentile_process(const Mat &input, Histogram &calc_hist, Mat &output) {
    Mat histogram;
    calc_hist(input, histogram);

    float number = static_cast<float>(input.cols * input.rows * PERCENTILE);
    float accum { 0.0f };

    int thresh { 0 };
    for (; thresh < HIST_SIZE; thresh++) {
        accum += histogram.at<float>(thresh);

        if (accum >= number) break;
    }

    ofile << thresh << endl;
    threshold(input, output, thresh, 255, THRESH_BINARY);
}

void run_percentile_segmentator() {
    string wd = "C:/Users/milena/git/PlantProject/Media/";

    VideoCapture video("C:/Users/milena/git/PlantProject/Media/climbing_bean_project3_leaf_folding.AVI");

    int  out_width = static_cast<int>(video.get(CAP_PROP_FRAME_WIDTH)) / 3;
    int out_height = static_cast<int>(video.get(CAP_PROP_FRAME_HEIGHT)) - BAR_HEIGHT;
    VideoWriter salida("C:/Users/milena/git/PlantProject/Media"
                      "/global_dump_out.AVI", static_cast<int>(video.get(CAP_PROP_FOURCC)),
                       video.get(CAP_PROP_FPS),
                       Size(out_width, out_height), false);

    ofile.open(WD + string("thresholds.txt"));

    Histogram hist;
    hist.set_hist_size(2);

    Mat frame;
    Mat output;

    for (pos = 0u; ; pos++, dump = (pos == NIGHT_SAMPLE || pos == DAY_SAMPLE) ) {
        video >> frame;

        if (pos == 1100 /*||frame.empty()*/) break;
        if (pos % 150 == 0)
            cout << "run_percentile_segmentator: analizados " + to_string(pos/30) + " segundos" << endl;

        preprocess(frame, frame);
        percentile_process(frame, hist, output);

        salida << output;
    }
}
