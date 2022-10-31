#include "BetterAnalyzer.h"

#include "common.h"

#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

// INPUT
static VideoCapture video;

// CONSTANTS
static constexpr double CONTRAST_FACTOR { 2.0 };
static constexpr double PERCENTILE { 0.206638 };
static constexpr int CONTOUR_MIN_SIZE { 5 };
static constexpr int STEM_WIDTH { 5 };

// DEBUG
static int pos;
static bool dump;

// CONTAINERS
static Mat input;
static Mat preproc_output;
static Mat thresh_output;
static Mat stem;
static Mat left_leaf;
static Mat right_leaf;
static Mat output;

// HISTOGRAM
static Histogram calc_hist;
static float plant_px_num;

// SEGMENTATION
static Point roi_corner;

// COLORS
static const Scalar white_1Cs { 255 };
static const Scalar red_s { 0, 0, 255 };

static const Vec3b red     {   0,   0, 255 };
static const Vec3b green   {   0, 255,   0 };
static const Vec3b yellow  {   0, 255, 255 };
static const Vec3b cyan    { 255, 255,   0 };

// MASKS
static Size roi_sz;
static Size half_roi;

//------------------------------------------------------------------------------
void initialize() {
    // Input initialization
    string file = wd + "climbing_bean_project3_leaf_folding.AVI";
    video.open(file);
    assert(video.isOpened());

    // Mask for histogram calculation
    Size sz(static_cast<int>(video.get(CAP_PROP_FRAME_WIDTH)),
            static_cast<int>(video.get(CAP_PROP_FRAME_HEIGHT)));
    Mat hist_mask = Mat::zeros(sz, CV_8UC1);
    Rect roi(hist_mask.cols/3, 0, hist_mask.cols/3, hist_mask.rows - BAR_HEIGHT);
    rectangle(hist_mask, roi, Scalar(255, 255, 255), FILLED);

    // Histogram calculation functor
    calc_hist.set_mask(hist_mask);
    plant_px_num = static_cast<float>(hist_mask.cols/3)
                 * static_cast<float>(hist_mask.rows - BAR_HEIGHT)
                 * static_cast<float>(PERCENTILE);

    // For leaf segmentation
    int cols = static_cast<int>(video.get(CAP_PROP_FRAME_WIDTH));
    int rows = static_cast<int>(video.get(CAP_PROP_FRAME_HEIGHT));

    roi_corner.x = cols / 3;
    roi_corner.y = rows / 3;

    roi_sz.width = cols/3;
    roi_sz.height = 2*rows/3 - BAR_HEIGHT;

    half_roi.width = roi_sz.width / 2;
    half_roi.height = roi_sz.height;

    TRACE(true, "* initialize(): roi(%d x %d)\n",
          roi_sz.width, roi_sz.height);
}
//------------------------------------------------------------------------------
void preprocess(const Mat &input, Mat &output) {
    // To grayscale
    cvtColor(input, output, COLOR_BGR2GRAY);
    // DUMP(dump, "Analyzer/gray_"+to_string(pos)+".png", output);

    // Contrast adjustment
    output *= CONTRAST_FACTOR;
    // DUMP(dump, "Analyzer/alpha_"+to_string(pos)+".png", output);

    TRACE(dump, "* preprocess(%d)\n", pos);
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
    DUMP(dump, "Analyzer/thresh_"+to_string(pos)+".png", output);

    TRACE(dump, "< percentile_threshold(%d)\n", pos);
}
//------------------------------------------------------------------------------
void color_stem(const Point &point, Mat &l_output, Mat &r_output) {
    int x, y;

    y = point.y + roi_corner.y;

    Scalar color = red;//white_1Cs;

    if (point.x != -1) {
        // If stem found, color over opposite side
        x = point.x + roi_corner.x;

        // Add stem thickness
        x -= STEM_WIDTH;
        // From point to end of row
        line(l_output, { x, y }, { l_output.cols, y }, color);

        // Add stem thickness
        x += 2 * STEM_WIDTH;
        // From beginning of row to point
        line(r_output, { 0, y }, { x, y }, color);
    } else {
        // Else, color over whole row
        line(l_output, { 0, y }, { l_output.cols, y }, color);
        line(r_output, { 0, y }, { r_output.cols, y }, color);
    }
}
//------------------------------------------------------------------------------
void leaf_segmentation(const Mat &input, Mat &l_output, Mat &r_output, Mat &stem) {
    TRACE(dump, "> leaf_segmentation(%d)\n", pos);
    bool found { false };
    int last { 0 }, i { 0 }, radius { 1 };

    auto is_stem = [](Vec3b px) -> bool {
        return px[0] == 0 && px[1] == 0 && px[2] == 0;
    };

    cvtColor(input, stem, COLOR_GRAY2BGR);
    cvtColor(input, r_output, COLOR_GRAY2BGR);
//    r_output = input.clone();
    cvtColor(input, l_output, COLOR_GRAY2BGR);
//    l_output = input.clone();

    // Reference to region of interest
    Rect mask(roi_corner, roi_sz);
    Mat ref = stem(mask);
    TRACE(dump, "* leaf_segmentation(%d): mask(%d x %d) ref(%d x %d)\n", pos,
          mask.width, mask.height,
          ref.cols, ref.rows);

    // To start, search whole row
    for (i = 0; !found && i < ref.rows; i++) {
        for (int j = 0; j < ref.cols; j++) {
            Vec3b &pixel = ref.at<Vec3b>(i, j);
            if ((found = is_stem(pixel))) {
                // Color stem red
                pixel = red;
                // Color over other leaf
                color_stem({j, i}, l_output, r_output);
                // Update last position
                last = j;
            } else {
                // Cover whole row
                color_stem({-1, i}, l_output, r_output);
            }
        }
    }

    TRACE(dump, "* leaf_segmentation(%d): first loop end, row = %d\n", pos, i);

    for (; i < ref.rows; i++) {
        found = false;
        // Search directly under last pos
        Vec3b &pixel = ref.at<Vec3b>(i, last);
        if ((found = pixel[0] == 0)) {
            // Stem color red
            pixel = red;
            // Color over other leaf
            color_stem({last, i}, l_output, r_output);
            continue;
        } else pixel = yellow;

        // Search near last
        for (int j = 1; !found && j <= radius; j++) {

            Vec3b &pixel1 = ref.at<Vec3b>(i, last - j);
            if ((found = pixel1[0] == 0)) {
                last = last - j;
                // Stem color red
                pixel1 = red;
                // Color over other leaf
                color_stem({last, i}, l_output, r_output);
                continue;
            } else pixel1 = cyan;

            Vec3b &pixel2 = ref.at<Vec3b>(i, last + j);
            if ((found = pixel2[0] == 0)) {
                last = last + j;
                // Stem color red
                pixel2 = red;
                // Color over other leaf
                color_stem({last, i}, l_output, r_output);
                continue;
            } else pixel2 = green;
        }

        if (! found) {
            radius += 2;
            // Cover whole row
            color_stem({-1, i}, l_output, r_output);
        } else radius = 1;
    }

    DUMP(dump, "Analyzer/stem_"+to_string(pos)+".png", stem);
    DUMP(dump, "Analyzer/left_"+to_string(pos)+".png", l_output);
    DUMP(dump, "Analyzer/right_"+to_string(pos)+".png", r_output);
    TRACE(dump, "< leaf_segmentation(%d)\n", pos);
}
//------------------------------------------------------------------------------
void ellipse_fitting(const Mat &input, bool left, Mat &output) {
    TRACE(dump, "> ellipse_fitting(%d, %s)\n", pos, left ? "left" : "right");
    vector<vector<Point>> contours;

    Mat input_ref, output_ref;
    if (left) {
        Rect mask(roi_corner, half_roi);
        rectangle(output, mask, Scalar(0));
        input_ref = input(mask);
        output_ref = output(mask);
    } else {
        Rect mask(Point(roi_corner.x + half_roi.width, roi_corner.y),
             half_roi);
        rectangle(output, mask, Scalar(0));
        input_ref = input(mask);
        output_ref = output(mask);
    }

    // Finds contours establishing a full hierarchy of nested contours, storing
    // both endpoints of each contour
    findContours(input_ref, contours, RETR_TREE, CHAIN_APPROX_SIMPLE);

    // For each contour, find and draw ellipse if big enough
    for (size_t i = 0; i < contours.size(); i++) {
        if (contours[i].size() > CONTOUR_MIN_SIZE) {
            RotatedRect ellipse_rect = fitEllipse(contours[i]);
            ellipse(output_ref, ellipse_rect, red);
        }
    }
    TRACE(dump, "< ellipse_fitting(%d, %s)\n", pos, left ? "left" : "right");
    DUMP(dump, "Analyzer/ellipses_"+to_string(pos)+".png", output_ref);
}
//------------------------------------------------------------------------------
void run_better_analyzer() {
    TRACE(true, "> run_better_analyzer\n");
    initialize();
    TRACE(true, "* run_better_analyzer(%d): roi(%d x %d) half_roi(%d x %d)\n", pos,
          roi_sz.width, roi_sz.height,
          half_roi.width, half_roi.height);

    for (pos = 0;; pos++) {
        video >> input;

        if (pos == 1051 || input.empty()) break;
        dump = (pos % 150 == 0);
        TRACE(dump, "* run_better_analyzer: %d segundos analizados\n", pos/30);

        // Preprocess image
        preprocess(input, preproc_output);

        // Thresholding
        percentile_threshold(preproc_output, thresh_output);

        // Segmentation
        leaf_segmentation(thresh_output, left_leaf, right_leaf, stem);

        // Ellipse fitting
//        cvtColor(thresh_output, output, COLOR_GRAY2BGR);
//        ellipse_fitting(left_leaf, true, output); pos--;
//        ellipse_fitting(right_leaf, false, output); pos++;

        // DUMP(dump, "Analyzer/ellipses_"+to_string(pos)+".png", output);
    }
    TRACE(true, "< run_better_analyzer\n");
}
//------------------------------------------------------------------------------
void test_line() {
//    Mat test = Mat::zeros(1080, 720, CV_8UC3);

//    // FILLED  RED
//    // LINE_4  YELLOW
//    // LINE_8
//    // LINE_AA GREEN

//    int x { 300 };
//    int y { 100 };

//    cout << "Doing LINE_4 line..." << endl;
//    for (y = 200; y < 205; x++, y++) {
//        line(test, { x, y }, { test.cols, y }, Scalar(255, 0, 255), 1, LINE_4);
//    }

//    cout << "Doing LINE_8 line..." << endl;
//    for (y = 300; y < 305; x++, y++) {
//        line(test, { x, y }, { test.cols, y }, Scalar(0, 255, 0), 1, LINE_8);
//    }

//    cout << "Doing LINE_AA line..." << endl;
//    for (y = 400; y < 405; x++, y++) {
//        line(test, { x, y }, { test.cols, y }, Scalar(0, 255, 255), 1, LINE_AA);
//    }

//    imshow("test", test);
//    waitKey();
}
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
 */
