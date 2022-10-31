#include "PercentileSegmentator.h"

#include "common.h"

#include <vector>
#include <stdio.h>
#include <fstream>

#include "GlobalSegmentator.h"

using namespace std;
using namespace cv;

extern string wd;
extern int DN_BREAKPOINTS[];

static int pos;
static bool dump;
static Histogram calc_hist;

static ofstream ofile;

// Full preprocessing
void percentile_preprocess(const Mat &src, Mat &dst) {

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
//------------------------------------------------------------------------------
// Preprocessing first half
void matrix_transform(const Mat &src, Mat &dst) {
    // Recortar
    crop_roi(src, dst);
    if (dump)
        save("Global/" + to_string(pos) + "_cropped", dst);

    // A escala de grises
    cvtColor(dst, dst, COLOR_RGB2GRAY);
    if (dump)
        save("Global/" + to_string(pos) + "_gs", dst);

    // moved from second half, necessary?
    // Aumentar contraste
    dst = dst * GLOBAL_SEGM_ALPHA;
    if (dump)
        save("Global/" + to_string(pos) + "_a", dst);
}
//------------------------------------------------------------------------------
// Preprocessing second half
void color_transform(const Mat &src, Mat &dst) {

    // Umbralizar
    double thr = 132.526;
    threshold(dst, dst, thr, 255, THRESH_BINARY);
    if (dump)
        save("Global/" + to_string(pos) + "_thr", dst);
}
//------------------------------------------------------------------------------
float calculate_percentile(Histogram &hist, const Mat &frame) {
    Mat histogram;
    hist(frame, histogram);

    float value = histogram.at<float>(0) / (frame.cols * frame.rows);

    if (dump)
        cout << "calculate_percentile: percentile = " << value << endl;

    return value;
}
//------------------------------------------------------------------------------
void determine_percentile() {
    VideoCapture video(wd + "climbing_bean_project3_leaf_folding.AVI");

    int  out_width = static_cast<int>(video.get(CAP_PROP_FRAME_WIDTH)) / 3;
    int out_height = static_cast<int>(video.get(CAP_PROP_FRAME_HEIGHT)) - BAR_HEIGHT;
    VideoWriter salida(wd + "/global_dump_out.AVI",
                       static_cast<int>(video.get(CAP_PROP_FOURCC)),
                       video.get(CAP_PROP_FPS),
                       Size(out_width, out_height), false);

    ofile.open(wd + "percentiles.txt");
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
            cout << "run_percentile_segmentator: analizados " +
                    to_string(pos/30) + " segundos" << endl;

        percentile_preprocess(frame, output);
        float num = calculate_percentile(hist, output);

        num_samples++;
        sum += num;

        salida << output;
    }

    cout << "run_percentile_segmentator: percentil = " << sum /num_samples << endl;
}
//------------------------------------------------------------------------------
void determine_percentile_dn() {
    VideoCapture video(wd + "climbing_bean_project3_leaf_folding.AVI");

    cout << "determine_percentile_dn: total_frames = " << video.get(CAP_PROP_FRAME_COUNT) <<
            endl; //aaaaa

    int  out_width = static_cast<int>(video.get(CAP_PROP_FRAME_WIDTH)) / 3;
    int out_height = static_cast<int>(video.get(CAP_PROP_FRAME_HEIGHT)) - BAR_HEIGHT;
    VideoWriter salida(wd + "/global_dump_out.AVI", static_cast<int>(video.get(CAP_PROP_FOURCC)),
                       video.get(CAP_PROP_FPS),
                       Size(out_width, out_height), false);

    ofile.open(wd + "percentiles.csv");

    int num_samples { 1 };

    float perc_sum { 0.0f };
    vector<float> average_percentile;

    int br_sum { 0 };
    vector<int> average_brightness;

    Histogram hist;
    hist.set_hist_size(2);

    Mat frame;
    Mat output;

    int bp;

    for (pos = 0u, bp = 0; ; pos++, dump = (pos == NIGHT_SAMPLE || pos == DAY_SAMPLE), num_samples++) {
        video >> frame;

        // Loop condition
        if (/*pos == 1100 || */frame.empty()) break;
        // Progress update
        if (pos % 150 == 0)
            cout << "determine_percentile_dn: analizados " + to_string(pos/30) + " segundos" << endl;

        matrix_transform(frame, output);

        // Brightness calc
        br_sum += static_cast<int>(mean(output)[0]);

        color_transform(output, output);

        // Percentile calc
        perc_sum += calculate_percentile(hist, output);

        salida << output;

        if (pos == DN_BREAKPOINTS[bp]) {
            cout << "determine_percentile_dn: breakpoint at " << pos
                 << "(num_samples = " << num_samples << ", p_sum = " << perc_sum
                 << ", b_sum = " << br_sum << ")" << endl;

            average_percentile.push_back(perc_sum / num_samples);
            average_brightness.push_back(br_sum / num_samples);
            perc_sum = 0.f; br_sum = 0; num_samples = 0;
            bp++;
        }
    }

    ofile << "breakpoint,average_brightness,average_percentile" << endl;
    for (uint i = 0u; i < ARRAY_SIZE(DN_BREAKPOINTS); i++) {
        ofile << DN_BREAKPOINTS[i] << "," << average_brightness[i] << ","
              << average_percentile[i] << endl;
        cout << DN_BREAKPOINTS[i] << " " << average_brightness[i] << " "
              << average_percentile[i] << endl;
    }

    cout << "determine_percentile_dn: ARRAY_SIZE = " << ARRAY_SIZE(DN_BREAKPOINTS) <<
            " vector_percentile_size = " << average_percentile.size() <<
            " vector_brightness_size = " << average_brightness.size() << endl;

}
//------------------------------------------------------------------------------
int percentile_process(const Mat &input, Mat &output) {
    matrix_transform(input, output);

    Mat histogram;
    calc_hist(output, histogram);

    float number = static_cast<float>(output.cols * output.rows * PERCENTILE);
    float accum { 0.0f };

    int thresh { 0 };
    for (; thresh < HIST_SIZE; thresh++) {
        accum += histogram.at<float>(thresh);

        if (accum >= number) break;
    }

    threshold(output, output, thresh, 255, THRESH_BINARY);

    return thresh;
}
//------------------------------------------------------------------------------
void run_percentile_segmentator() {
    string input = wd + "climbing_bean_project3_leaf_folding.AVI";
    VideoCapture video(input);
    if (! video.isOpened()) {
        cerr << "Could not open input video: " << input << endl;
        return;
    }

    int  out_width = static_cast<int>(video.get(CAP_PROP_FRAME_WIDTH)) / 3;
    int out_height = static_cast<int>(video.get(CAP_PROP_FRAME_HEIGHT)) - BAR_HEIGHT;
    VideoWriter salida(wd + "global_dump_out.AVI", static_cast<int>(video.get(CAP_PROP_FOURCC)),
                       video.get(CAP_PROP_FPS),
                       Size(out_width, out_height), false);

    ofile.open(WD + string("thresholds.txt"));

    Mat frame;
    Mat output;

    for (pos = 0u; ; pos++, dump = (pos == NIGHT_SAMPLE || pos == DAY_SAMPLE) ) {
        video >> frame;

        if (/*pos == 1100 || */frame.empty()) break;
        if (pos % 150 == 0)
            cout << "run_percentile_segmentator: analizados " + to_string(pos/30) + " segundos" << endl;

        percentile_process(frame, output);

        salida << output;
    }
}
