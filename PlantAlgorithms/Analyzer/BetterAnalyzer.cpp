#include "BetterAnalyzer.h"

#include "common.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

#define VIDEO_OUTPUT

// INPUT
static VideoCapture video;

// VIDEO OUTPUT
static int frame_ms;
static vector<string> win_name {
    "stem",
    "left_leaf",
    "right_leaf",
    "left_ellipses",
    "right_ellipses"
};
static vector<Point> win_pos {
    Point(     0,      0),
    Point(    10,     10),
#ifdef DESKTOP
    Point(960+20,     10),
    Point(    10, 540+20),
    Point(960+20, 540+20)
#else
    Point(540+20,     10),
    Point(    10, 480+20),
    Point(540+20, 480+20)
#endif
};

// DATA OUTPUT
static ofstream left_leaf_of;
static ofstream right_leaf_of;
static vector<float> left_leaf_vector;
static vector<float> right_leaf_vector;
static Mat left_plot;
static Mat right_plot;

// CONTAINERS
static Mat input;
static Mat preproc_output;
static Mat thresh_output;
static Mat stem;
static Mat left_leaf;
static Mat right_leaf;
static Mat l_output;
static Mat r_output;

// CONSTANTS
static constexpr double CONTRAST_FACTOR  { 2.0 };
static constexpr double PERCENTILE       { 0.206638 };
static constexpr    int CONTOUR_MIN_SIZE { 15 };
static constexpr    int STEM_WIDTH       { 5 };
static constexpr  uchar LS_COLOR         { 255 };

// DEBUG
static int pos;
static bool dump;

// HISTOGRAM
static Histogram calc_hist;
static float plant_px_num;

// SEGMENTATION
static Point roi_corner;

// COLORS
static const Vec3b v_red     {   0,   0, 255 };
static const Vec3b v_green   {   0, 255,   0 };
static const Vec3b v_magenta {  255,  0, 255 };
static const Vec3b v_cyan    { 255, 255,   0 };

// MASKS
static Size roi_sz;

//------------------------------------------------------------------------------
extern template void plot_vector<float>(const std::vector<float>, cv::Mat &);
//------------------------------------------------------------------------------
void initialize() {
    // Input initialization
    string file = wd + "climbing_bean_project3_leaf_folding.AVI";
    video.open(file);
    assert(video.isOpened());

    // ROI
    Size sz(static_cast<int>(video.get(CAP_PROP_FRAME_WIDTH)),
            static_cast<int>(video.get(CAP_PROP_FRAME_HEIGHT)));

    roi_corner.x = sz.width / 3;
    roi_corner.y = sz.height / 3;

    roi_sz.width  = sz.width / 3;
    roi_sz.height = 2 * sz.height / 3 - BAR_HEIGHT;

    // Mask for histogram calculation
    Mat hist_mask = Mat::zeros(sz, CV_8UC1);
    Rect roi(roi_corner, roi_sz);
    rectangle(hist_mask, roi, Scalar(255, 255, 255), FILLED);

    calc_hist.set_mask(hist_mask);

    // Percentile Threshold
    plant_px_num = static_cast<float>(sz.width / 3)
                 * static_cast<float>(sz.height - BAR_HEIGHT)
                 * static_cast<float>(PERCENTILE);

    // For output video feed
    frame_ms = static_cast<int>(1000.0 / video.get(CAP_PROP_FPS));

    // File output init
    left_leaf_of.open( DUMP_WD + string("Analyzer/data_left.csv"),
                       ofstream::out | ofstream::trunc);
    right_leaf_of.open(DUMP_WD + string("Analyzer/data_right.csv"),
                       ofstream::out | ofstream::trunc);

    TRACE(true, "* initialize(): roi(%d x %d)\n",
          roi_sz.width, roi_sz.height);
    TRACE(true, "* initialize(): frame_ms = %d\n", frame_ms);
}
//------------------------------------------------------------------------------
void init_windows() {
    Size sz(static_cast<int>(video.get(CAP_PROP_FRAME_WIDTH)),
            static_cast<int>(video.get(CAP_PROP_FRAME_HEIGHT)));
    Size win_sz(static_cast<int>(sz.width / 1.5),
                static_cast<int>(sz.height / 1.5));

    for(size_t i = 0; i < win_name.size(); i++) {
        namedWindow(win_name[i], WINDOW_NORMAL | WINDOW_KEEPRATIO);
        resizeWindow(win_name[i], win_sz);
        moveWindow(win_name[i], win_pos[i].x, win_pos[i].y);
    }
}
//------------------------------------------------------------------------------
void preprocess(const Mat &input, Mat &output) {
    // To grayscale
    cvtColor(input, output, COLOR_BGR2GRAY);

    // Contrast adjustment
    output *= CONTRAST_FACTOR;

    TRACE(dump, "* preprocess(%d)\n", pos);
}
//------------------------------------------------------------------------------
void close(int size, Mat &image) {
    Mat element = getStructuringElement(MORPH_RECT, Size(size, size));
    morphologyEx(image, image, MORPH_CLOSE, element);
}
//------------------------------------------------------------------------------
void percentile_threshold(const Mat &input, Mat &output) {
    TRACE(dump, "> percentile_threshold(%d)\n", pos);

    Mat histogram;
    calc_hist(input, histogram);

    float accum { 0.0f };
    int thresh { 0 };

    for (; thresh < HIST_SIZE; thresh++) {
        accum += histogram.at<float>(thresh);

        if (accum >= plant_px_num) break;
    }

    threshold(input, output, thresh, 255, THRESH_BINARY);
    TRACE(dump, "* percentile_threshold(%d): threshold = %d\n", pos, thresh);

    DUMP(dump, output, "Analyzer/thresh_%d.png", pos);

    TRACE(dump, "< percentile_threshold(%d)\n", pos);
}
//------------------------------------------------------------------------------
void color_leaves(const Point &point, Mat &l_output, Mat &r_output) {

    Scalar color = { 255 };

    if (point.x != -1) {
        // If stem found, color over opposite side
        // From point to end of row
        line(l_output, { point.x, point.y }, { l_output.cols, point.y }, color);
        // From beginning of row to point
        line(r_output, { 0, point.y }, { point.x, point.y }, color);
    } else {
        // Else, color over whole row
        line(l_output, { 0, point.y }, { l_output.cols, point.y }, color);
        line(r_output, { 0, point.y }, { r_output.cols, point.y }, color);
    }
}
//------------------------------------------------------------------------------
void hide_plant_pot(Mat &output) {
    Rect pot(0, POT_Y_POS, output.cols, output.rows - POT_Y_POS - BAR_HEIGHT);
    rectangle(output, pot, LS_COLOR, FILLED);
}
//------------------------------------------------------------------------------
void leaf_segmentation(const Mat &input, Mat &l_output, Mat &r_output, Mat &stem) {
    TRACE(dump, "> leaf_segmentation(%d)\n", pos);

    static constexpr int MAX_WIDTH { 6 };

    // Whether any stem px has been found
    bool found { false };
    // Whether next stem px to be found is first
    bool first { true };
    // Position of first stem px found in last row
    int last { 0 };
    // Current row index
    int i { 0 };
    // Search radius
    int radius { 1 };

    auto is_stem = [](Vec3b px) -> bool {
        return px[0] == 0 && px[1] == 0 && px[2] == 0;
    };

    // Copy input to output images
    cvtColor(input, stem, COLOR_GRAY2BGR);
    // cvtColor(input, r_output, COLOR_GRAY2BGR);
    // cvtColor(input, l_output, COLOR_GRAY2BGR);
    r_output = input.clone();
    l_output = input.clone();

    // Reference to region of interest
    Rect mask(roi_corner, roi_sz);
    rectangle(stem, mask, v_red);

    Mat ref = stem(mask);
    Mat l_ref = l_output(mask);
    Mat r_ref = r_output(mask);

    TRACE(dump, "* leaf_segmentation(%d): mask(%d x %d) ref(%d x %d)\n", pos,
          mask.width, mask.height,
          ref.cols, ref.rows);

    TRACE(dump, "* leaf_segmentation(%d): first loop start\n", pos);
    // To start, search whole row
    for (i = 0; !found && first && i < ref.rows; i++) {
        for (int j = 0; j < ref.cols; j++) {
            Vec3b &pixel = ref.at<Vec3b>(i, j);
            if ((found = is_stem(pixel))) {
                // Color stem red
                pixel = v_red;
                TRACE(dump, "* leaf_segmentation(%d): pixel found: (%d, %d)\n", pos, i, j);
                if (first) {
                    TRACE(dump, "* leaf_segmentation(%d): it's the first!!\n", pos);
                    // Color over leaves
                    color_leaves({j, i}, l_ref, r_ref);
                    // Update last position
                    last = j;
                    // Next stem pxs found are not first
                    first = false;
                } else {
                    TRACE(dump, "* leaf_segmentation(%d): not the first\n", pos);
                    // Color over stem px in r_ref
                    r_ref.at<uchar>(i, j) = LS_COLOR;
                    // for l_ref, color_leaves covers all stem pxs
                }
            } else {
                // Found end of stem (first stem px has been found before)
                if (!first) {
                    TRACE(dump, "* leaf_segmentation(%d): first has been found before, therefore end of stem\n", pos);
                    break;
                }
            }
        }
        if (first) { // First not found
            TRACE(dump, "* leaf_segmentation(%d): end of row, first pixel not found: (%d)\n", pos, i);
            // Cover whole row
            color_leaves({-1, i}, l_ref, r_ref);
        }
    }

    TRACE(dump, "* leaf_segmentation(%d): first loop end, row = %d\n", pos, i);

    for (; i < ref.rows; i++) {
        found = false;

        // Search directly under last pos
        Vec3b &pixel = ref.at<Vec3b>(i, last);
        if ((found = is_stem(pixel))) {
            // Stem color red
            pixel = v_red;
            // Color over leaves
            color_leaves({last, i}, l_ref, r_ref);

            // Keep searching left and right
            bool l { true }, r { true };
            for (int k = 1; (l || r) && k < MAX_WIDTH; k++) {
                if (l) {
                    Vec3b &px_l = ref.at<Vec3b>(i, last - k);
                    if (is_stem(px_l)) {
                        // Color stem red
                        px_l = v_red;
                        // Color over stem px in leaf output
                        l_ref.at<uchar>(i, last - k) = LS_COLOR;
                        r_ref.at<uchar>(i, last - k) = LS_COLOR;
                    }
                    else l = false; // stop searching left
                }
                if (r) {
                    Vec3b &px_r = ref.at<Vec3b>(i, last + k);
                    if (is_stem(px_r)) {
                        // Color stem red
                        px_r = v_red;
                        // Color over stem px in leaf output
                        l_ref.at<uchar>(i, last + k) = LS_COLOR;
                        r_ref.at<uchar>(i, last + k) = LS_COLOR;
                    }
                    else r = false; // stop searching right
                }
            }

            continue; // next row
        } else pixel = v_magenta;

        // Search near last
        for (int j = 1; !found && j <= radius; j++) {

            // To the left
            Vec3b &px_l = ref.at<Vec3b>(i, last - j);
            if ((found = is_stem(px_l))) {
                last = last - j;
                // Stem color red
                px_l = v_red;
                // Color over leaves
                color_leaves({last, i}, l_ref, r_ref);
                // Keep searching left
                for (int k = 1; k < MAX_WIDTH; k++) {
                    Vec3b &px = ref.at<Vec3b>(i, last - k);
                    if (is_stem(px)){
                        // Color stem red
                        px = v_red;
                        // Color over stem px in left leaf output
                        l_ref.at<uchar>(i, last - k) = LS_COLOR;
                    } else break; // stop searching left
                }
                continue; // next row
            } else px_l = v_cyan;

            // To the right
            Vec3b &px_r = ref.at<Vec3b>(i, last + j);
            if ((found = is_stem(px_r))) {
                last = last + j;
                // Stem color red
                px_r = v_red;
                // Color over other leaf
                color_leaves({last, i}, l_ref, r_ref);
                // Keep searching right
                for (int k = 1; k < MAX_WIDTH; k++) {
                    Vec3b &px = ref.at<Vec3b>(i, last + k);
                    if (is_stem(px)){
                        // Color stem red
                        px = v_red;
                        // Color over stem px in right leaf output
                        r_ref.at<uchar>(i, last + k) = LS_COLOR;
                    } else break; // stop searching right
                }
                continue; // next row
            } else px_r = v_green;
        }

        // For rows where stem disappears, not found after searching within radius
        if (! found) {
            radius += 2;
            // Cover whole row
            // color_leaves({-1, i}, l_ref, r_ref);
        } else
            // Stem found again
            radius = 1; // maybe 2?
    }

    hide_plant_pot(l_output);
    hide_plant_pot(r_output);

    DUMP(dump, stem, "Analyzer/stem_%d.png", pos);
    DUMP(dump, l_output, "Analyzer/left_%d.png", pos);
    DUMP(dump, r_output, "Analyzer/right_%d.png", pos);
    TRACE(dump, "< leaf_segmentation(%d)\n", pos);
}
//------------------------------------------------------------------------------
void ellipse_fitting(const Mat &input, bool left, Mat &output) {
    TRACE(dump, "> ellipse_fitting(%d, %s)\n", pos, left ? "left" : "right");
    vector<vector<Point>> contours;
    int c_drawn { 0 };

    Mat input_ref, output_ref, contours_out;
    Rect mask(roi_corner, roi_sz);
    input_ref = input(mask);
    output_ref = output(mask);
    contours_out = output_ref.clone();

    // Finds contours establishing a full hierarchy of nested contours, storing
    // both endpoints of each contour
    findContours(input_ref, contours, RETR_TREE, CHAIN_APPROX_SIMPLE);
    drawContours(contours_out, contours, -1, { 0, 0, 255 });

    size_t max { 0 }, index { 0 };

    // For each contour, find and draw ellipse if big enough
    for (size_t i = 0; i < contours.size(); i++) {
        if (contours[i].size() > max) { // contours[i].size() > CONTOUR_MIN_SIZE
            max = contours[i].size();
            index = i;
        }
    }
    RotatedRect ellipse_rect = fitEllipse(contours[index]);
    ellipse(output_ref, ellipse_rect, v_red);

    (left ? left_leaf_vector : right_leaf_vector).push_back(ellipse_rect.angle);

    TRACE(c_drawn > 2, "* ellipse_fitting(%d, %s): contours drawn = %d\n", pos,
         (left ? "left" : "right"), c_drawn);
    TRACE(dump, "< ellipse_fitting(%d, %s)\n", pos, left ? "left" : "right");
    DUMP(dump, output/*_ref*/, "Analyzer/ellipses_%d_%s.png", pos, (left ? "left" : "right"));
    DUMP(dump, contours_out, "Analyzer/contours_%d_%s.png", pos, (left ? "left" : "right"));
}
//------------------------------------------------------------------------------
void dump_data() {
    uint limit = min(left_leaf_vector.size(), right_leaf_vector.size());

    for (uint i = 0; i < limit; i++) {
        left_leaf_of << left_leaf_vector[i] << endl;
        right_leaf_of << right_leaf_vector[i] << endl;
    }
}
//------------------------------------------------------------------------------
void run_better_analyzer() {
    TRACE(true, "> run_better_analyzer\n");
    initialize();
#ifdef VIDEO_OUTPUT
    init_windows();
#endif
    TRACE(true, "* run_better_analyzer(%d): roi(%d x %d)\n", pos,
          roi_sz.width, roi_sz.height);

    for (pos = 0;; pos++) {
        video >> input;

        if (/*pos == 1051 ||*/ input.empty()) break;
        dump = (pos % 150 == 0 || pos == 49);
        TRACE(dump, "* run_better_analyzer: %d segundos analizados\n", pos/30);

        // Preprocess image
        preprocess(input, preproc_output);

        // Thresholding
        percentile_threshold(preproc_output, thresh_output);

        // Segmentation
        leaf_segmentation(thresh_output, left_leaf, right_leaf, stem);

        // Ellipse fitting
        close(5, left_leaf); // clean to avoid unwanted contours
        close(5, right_leaf); // clean to avoid unwanted contours
        cvtColor(thresh_output, l_output, COLOR_GRAY2BGR);
        cvtColor(thresh_output, r_output, COLOR_GRAY2BGR);
        ellipse_fitting(left_leaf, true, l_output);
        ellipse_fitting(right_leaf, false, r_output);

        // Output video feed
        #ifdef VIDEO_OUTPUT
        imshow("stem", stem);
        imshow("left_leaf", left_leaf);
        imshow("right_leaf", right_leaf);
        imshow("left_ellipses", l_output);
        imshow("right_ellipses", r_output);
        waitKey(frame_ms);
        #endif
    }

    // Dump data to file
    DUMP(true, left_plot, "Analyzer/plot_left.png");
    DUMP(true, right_plot, "Analyzer/plot_right.png");

    dump_data();
    destroyAllWindows();
    TRACE(true, "< run_better_analyzer()\n");
}
//------------------------------------------------------------------------------
void run_better_analyzer_del_stem() {
    TRACE(true, "> run_better_analyzer\n");
    initialize();
    TRACE(true, "* run_better_analyzer(%d): roi(%d x %d)\n", pos,
          roi_sz.width, roi_sz.height);

    for (pos = 0;; pos++) {
        video >> input;

        if (pos == 1051 || input.empty()) break;
        dump = (pos % 150 == 0 || pos == 49);
        TRACE(dump, "* run_better_analyzer: %d segundos analizados\n", pos/30);

        // Preprocess image
        preprocess(input, preproc_output);

        // Thresholding
        percentile_threshold(preproc_output, thresh_output);
        close(7, thresh_output);
        hide_plant_pot(thresh_output);

        // Ellipse fitting
        cvtColor(thresh_output, l_output, COLOR_GRAY2BGR);
        ellipse_fitting(thresh_output, true, l_output);

        // Output video feed
#ifdef VIDEO_OUTPUT
        imshow("left_ellipses", l_output);
        waitKey(frame_ms);
#endif
    }

    destroyAllWindows();
    TRACE(true, "< run_better_analyzer()\n");
}
//------------------------------------------------------------------------------
void dump_frame(int pos) {
    // Input initialization
    string file = wd + "climbing_bean_project3_leaf_folding.AVI";
    video.open(file);
    assert(video.isOpened());

    video.set(CAP_PROP_POS_FRAMES, pos);
    Mat frame;
    video >> frame;
    DUMP(true, frame, "frame_%d.png", pos);
}
//------------------------------------------------------------------------------
/*
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */
