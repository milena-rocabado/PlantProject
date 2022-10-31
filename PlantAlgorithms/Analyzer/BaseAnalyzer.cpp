 #include "BaseAnalyzer.h"

#include <string>
#include <vector>

#include "common.h"
#include "PercentileSegmentator.h"

using namespace std;
using namespace cv;

extern string wd;
static uint pos;
static bool dump;

static vector<int> stem;
static Mat right_leaf, left_leaf;

void analyzer_preprocess(const Mat &input, Mat &output) {
    crop_upper_third(input, output);
    cvtColor(output, output, COLOR_GRAY2BGR);
}

void stem_process(const Mat &input) {
    assert(static_cast<uint>(input.rows) == stem.size());

    right_leaf = input.clone();
    left_leaf = input.clone();

//    int row = 0;
//    for (int col : stem) {
//        Point s { col, row };
//        Point a { 0, row }, b { row, input.cols };
//        TRACE(dump, "stem_process: s(%d, %d) a(%d, %d) b(%d, %d)\n",
//              s.x, s.y,
//              a.x, a.y,
//              b.x, b.y);
////        Point s { row, col };
////        Point a { row, 0 }, b { input.cols, row };
//        line(right_leaf, s, b, Scalar(0, 0, 255));
//        line(left_leaf, a, s, Scalar(0, 0, 255));
//        row++;
//    } TRACE(dump, "\n");

    for (int row = 0 ; row < input.rows; row++) {
        int col = stem[row];

        for (int i = 0; i <= col; i++) {
            Vec3b &pixel = left_leaf.at<Vec3b>(row, i);
            pixel = { 0, 0, 255 };
        }

        for (; col < input.cols; col++) {
            Vec3b &pixel = right_leaf.at<Vec3b>(row, col);
            pixel = { 0, 0, 255 };
        }
    }

    if (dump) {
        save("Analyzer/right-" + to_string(pos), right_leaf);
        save("Analyzer/left-" + to_string(pos), left_leaf);
    }
}

void analyzer_process(const Mat &input, Mat &output) {
    vector<int> failed_rows;
    bool found { false };
    int last { 0 }, i { 0 }, radius { 1 };

    auto color = [](Vec3b &pixel, char c) {
        if (! dump) return;
        switch(c) {
        case 'r': pixel = {   0,   0, 255 }; break;
        case 'g': pixel = {   0, 255,   0 }; break;
        case 'c': pixel = { 255, 255,   0 }; break;
        case 'y': pixel = {   0, 255, 255 }; break;
        case 'x': pixel = { 255,   0, 255 }; break;
        }
    };

    analyzer_preprocess(input, output);

    for (i = 0; !found && i < output.rows; i++) {
        for (int j = 0; j < output.cols; j++) { // search whole row
            Vec3b &pixel = output.at<Vec3b>(i, j);
            if ((found = pixel[0] == 0)) {
                color(pixel, 'r');
                stem.push_back(j);
                last = j;
                break;
            }
        }
        if (! found) {
            failed_rows.push_back(i);
            stem.push_back(output.rows / 2);
        }
    }

    TRACE(dump, "analyzer_process(%d): first row = %d, last = %d\n", pos, i, last);

    for (; i < output.rows; i++) {
        found = false;
        // search directly under
        Vec3b &pixel = output.at<Vec3b>(i, last);
        if ((found = pixel[0] == 0)) {
            color(pixel, 'r');
            stem.push_back(last);
            continue;
        } else color(pixel, 'y');

        // search near last
        for (int j = 1; !found && j <= radius; j++) {

            Vec3b &pixel1 = output.at<Vec3b>(i, last - j);
            if ((found = pixel1[0] == 0)) {
                color(pixel1, 'r');
                last = last - j;
                stem.push_back(last);
                continue;
            } else color(pixel1, 'c');

            Vec3b &pixel2 = output.at<Vec3b>(i, last + j);
            if ((found = pixel2[0] == 0)) {
                color(pixel2, 'r');
                last = last + j;
                stem.push_back(last);
                continue;
            } else color(pixel2, 'x');
        }

        if (! found) {
            failed_rows.push_back(i);
            stem.push_back(last);
            radius += 2;
        } else {
            radius = 1;
        }
    }

    TRACE(dump, "analyzer_process(%d): stem length = %d failed rows = %d frame height = %d\n",
          pos,
          stem.size(),
          failed_rows.size(),
          output.rows);

    stem_process(output);

    stem.clear();
}

void run_base_analyzer() {
    string input = wd + "climbing_bean_project3_leaf_folding.AVI";
    VideoCapture video(input);
    if (! video.isOpened()) {
        cerr << "Could not open input video: " << input << endl;
        return;
    }

    int  out_width = static_cast<int>(video.get(CAP_PROP_FRAME_WIDTH)) / 3;
    int out_height = static_cast<int>(video.get(CAP_PROP_FRAME_HEIGHT)) - BAR_HEIGHT;
    VideoWriter mask_out(wd + "mask_dump_out.AVI", static_cast<int>(video.get(CAP_PROP_FOURCC)),
                       video.get(CAP_PROP_FPS),
                       Size(out_width, out_height), false);

    out_height = 2 * out_height / 3;
    VideoWriter out(wd + "analyzer_dump_out.AVI", static_cast<int>(video.get(CAP_PROP_FOURCC)),
                       video.get(CAP_PROP_FPS),
                       Size(out_width, out_height), true);
    VideoWriter right_out(wd + "analyzer_right_out.AVI", static_cast<int>(video.get(CAP_PROP_FOURCC)),
                       video.get(CAP_PROP_FPS),
                       Size(out_width, out_height), true);
    VideoWriter left_out(wd + "analyzer_left_out.AVI", static_cast<int>(video.get(CAP_PROP_FOURCC)),
                       video.get(CAP_PROP_FPS),
                       Size(out_width, out_height), true);

    Mat frame, mask, output;
    // dump = (pos == 0 || pos == NIGHT_SAMPLE || pos == DAY_SAMPLE)
    int thresh { 0 };
    for (pos = 0u; ; pos++) {
        video >> frame;

        if (pos == 1051 /*|| frame.empty()*/) break;
        if ((dump = pos % 150 == 0))
            cout << "run_base_analyzer: " + to_string(pos/30) + " segundos analizados" << endl;

        thresh = percentile_process(frame, mask);

        mask_out << mask;

        analyzer_process(mask, output);

        out << output;
        right_out << right_leaf;
        left_out << left_leaf;

        if (dump) {
            cout << "run_base_analyzer: Threshold for " << pos << ": " << thresh << endl;
            save("Analyzer/mask-" + to_string(pos), mask);
            save("Analyzer/stem-" + to_string(pos), output);
        }
    }
}
