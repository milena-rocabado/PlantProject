#include "BetterAnalyzer.h"

#include "common.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <opencv2/opencv.hpp>
#include <stdlib.h>

using namespace std;
using namespace cv;

//#define VIDEO_OUTPUT
//#define OTSU
#define INITIAL_POS 0

// INPUT
static VideoCapture video;

// VIDEO OUTPUT
static int frame_ms;
static vector<string> win_name {
    "stem",
    "left_leaf",
    "right_leaf",
    "left_ellipses",
    "right_ellipses",
    "thresholding"
};

//static vector<Point> win_pos {
//    Point(     0,      0),
//    Point(    10,     10),
//#ifdef DESKTOP
//    Point(950+10,     10),
//    Point(    10, 530+10),
//    Point(950+20, 530+10),
//    Point(   950,    530)
//#else
//    Point(540+20,     10),
//    Point(    10, 480+20),
//    Point(540+20, 480+20)
//#endif
//};

static vector<Point> win_pos {
    Point(    510,   10),
    Point(    10,     10),
#ifdef DESKTOP
    Point(950+10,     10),
    Point(    10, 530+10),
    Point(950+20, 530+10),
    Point(510,    530+10)
#else
    Point(540+20,     10),
    Point(    10, 480+20),
    Point(540+20, 480+20)
#endif
};

// CONTAINERS
static Mat input;
// - Preprocessing
static Mat preproc_output;
static Mat stretch_output;
static Mat stretch_hist_output;
// - Thresholding
static Mat thresh_output;
// - Leaf segmentation
static Mat stem;
static Mat left_leaf;
static Mat right_leaf;
// - Final output
static Mat l_output;
static Mat r_output;

// CONSTANTS
// - Not used
static constexpr double CONTRAST_FACTOR  { 2.0 };
static constexpr double PERCENTILE       { 0.206638 };
static constexpr    int CONTOUR_MIN_SIZE { 15 };
static constexpr    int STEM_WIDTH       { 5 };
// - Leaf segmentation
static constexpr  uchar LS_COLOR         { 255 };
// - Finding threshold
static constexpr    int VALLEY_THRESHOLD { 130 };
static constexpr    int VALLEY_STREAK { 25 };
// - Day/night detection
static constexpr double TOLERANCE { 7. };
static constexpr int TIME_STREAK { 5 };
static constexpr int MIN_LENGTH { 100 };

// DEBUG
static int pos;
static bool dump;

// HISTOGRAM
static Histogram calc_hist;
static float plant_px_num;

// THRESHOLDING
static double optimal_threshold;

// COLORS
static const Vec3b v_red     {   0,   0, 255 };
static const Vec3b v_green   {   0, 255,   0 };
static const Vec3b v_magenta {  255,  0, 255 };
static const Vec3b v_cyan    { 255, 255,   0 };

// MASK
static Point roi_corner;
static Size roi_sz;
static Mat roi_mask;

// DAY/NIGHT DETECTION
static bool night { true };
static vector<int> breakpoints; // Stores frame first position of each period
static double last_br;
static    int last_pos;
static    int dif_streak { 0 };
static vector<double> br_plot;

// DATA OUTPUT
static ofstream data_of;
static vector<float> left_leaf_vector;
static vector<float> right_leaf_vector;
typedef struct {
    char padding[3];
    bool night;
    int pos;
    float left_angle;
    float right_angle;
} data_out_t;
static constexpr int DATA_OUT_NUM { TIME_STREAK };
static data_out_t data_out[DATA_OUT_NUM];
/*
 * Idea: ir almacenando info en estructura, cuando se llene volcar. La deteccion
 * de un breakpoint se retrasa maximo TIME_STREAK, por tanto almacenar TIME_STREAK
 * datos seria suficiente. Al encontrar bp, actualizar estructura.
 * Funcion para volcar info, funcion para actualizar info.
*/

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
    roi_mask = Mat::zeros(sz, CV_8UC1);
    Rect roi(roi_corner, roi_sz);
    rectangle(roi_mask, roi, Scalar(255, 255, 255), FILLED);

    calc_hist.set_mask(roi_mask);

    // Percentile Threshold
    plant_px_num = static_cast<float>(sz.width / 3)
                 * static_cast<float>(sz.height - BAR_HEIGHT)
                 * static_cast<float>(PERCENTILE);

    // For output video feed
    frame_ms = static_cast<int>(1000.0 / video.get(CAP_PROP_FPS));

    // File output init
    data_of.open(DUMP_WD + string("Analyzer/data_output.csv"),
                       ofstream::out | ofstream::trunc);
    data_of << "Frame,Day/Night,Left_leaf_angle,Right_leaf_angle" << endl;

    TRACE(true, "* initialize(): roi(%d x %d)\n",
          roi_sz.width, roi_sz.height);
    TRACE(true, "* initialize(): frame_ms = %d\n", frame_ms);
}
//------------------------------------------------------------------------------
void init_windows() {
//    Size sz(static_cast<int>(video.get(CAP_PROP_FRAME_WIDTH)),
//            static_cast<int>(video.get(CAP_PROP_FRAME_HEIGHT)));
//    Size win_sz(static_cast<int>(sz.width / 1.5),
//                static_cast<int>(sz.height / 1.5));

    Size win_sz = roi_sz;

    for(size_t i = 0; i < win_name.size(); i++) {
        namedWindow(win_name[i], WINDOW_NORMAL | WINDOW_KEEPRATIO);
        resizeWindow(win_name[i], win_sz);
        moveWindow(win_name[i], win_pos[i].x, win_pos[i].y);
    }
}
//------------------------------------------------------------------------------
void dump_out_data() {
    for (int i = 0; i < DATA_OUT_NUM; i++) {
        data_of << data_out[i].pos << ","
                << (data_out[i].night ? "night," : "day,")
                << data_out[i].left_angle << ","
                << data_out[i].right_angle << endl;
    }
}
//------------------------------------------------------------------------------
void update_data(int pos) {
    night = !night;
    for (int i = 0; i < DATA_OUT_NUM; i++) {
        if (data_out[i].pos >= pos) {
            data_out[i].night = night;
        }
    }
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void stretch_histogram_minmax(const Mat &input, Mat &output) {
    TRACE(dump, "> stretch_histogram_minmax(%d)\n", pos);

    Mat in_hist;
    calc_hist(input, in_hist);
    DUMP_HIST(dump, in_hist, "Analyzer/stretch_hist_in_%d.png", pos);

    double min, max;
    minMaxLoc(input, &min, &max, nullptr, nullptr, roi_mask);
    TRACE(dump, "* stretch_histogram_minmax(%d):  (in) min = %d max = %d\n", pos,
          static_cast<int>(min), static_cast<int>(max));

    input.convertTo(output, CV_32FC1);
    output -= min;
    output *= (255.0 / (max - min));

    minMaxLoc(output, &min, &max, nullptr, nullptr, roi_mask);
    TRACE(dump, "* stretch_histogram_minmax(%d): (out) min = %d max = %d\n", pos,
          static_cast<int>(min), static_cast<int>(max));

    Mat out_hist;
    calc_hist(output, out_hist);
    DUMP_HIST(dump, out_hist, "Analyzer/stretch_hist_out_mm_%d.png", pos);
//    TRACE(dump, "* stretch_histogram(%d): hist[min] = %d hist[max] = %d\n", pos,
//          static_cast<int>(out_hist.at<float>(static_cast<int>(min))),
//          static_cast<int>(out_hist.at<float>(static_cast<int>(max))));

    output.convertTo(output, CV_8UC1);
    TRACE(dump, "< stretch_histogram_minmax(%d)\n", pos);
}
//------------------------------------------------------------------------------
void stretch_histogram(const Mat &input, Mat &out_hist, Mat &output) {
    TRACE(dump, "> stretch_histogram(%d)\n", pos);
    static const float PERCENTILE = 0.015f;
    static const float NUM_PIXELS = static_cast<float>(input.cols)
                                  * static_cast<float>(input.rows)
                                  * PERCENTILE;

    Mat in_hist;
    calc_hist(input, in_hist);
    DUMP_HIST_N(dump, in_hist, static_cast<int>(optimal_threshold),
                "Analyzer/stretch_hist_in_%d.png", pos);

    double min, max;
    float num;
    int index;
    // max is top 5%
    for (index = HIST_SIZE - 1, num = .0f; index > 0; index--) {
        num += in_hist.at<float>(index);
        if (num > NUM_PIXELS) break;
    }
    max = static_cast<double>(index);
    // min is bottom 5%
    for (index = 0, num = .0f; index < HIST_SIZE; index++) {
        num += in_hist.at<float>(index);
        if (num > NUM_PIXELS) break;
    }
    min = static_cast<double>(index);

    TRACE(dump, "* stretch_histogram(%d):  (in) min = %d max = %d\n", pos,
          static_cast<int>(min), static_cast<int>(max));

    input.convertTo(output, CV_32FC1);
    output -= min;
    output *= (255.0 / (max - min));

    // minMaxLoc(output, &min, &max, nullptr, nullptr, roi_mask);
    // TRACE(dump, "* stretch_histogram(%d): (out) min = %d max = %d\n", pos,
    //       static_cast<int>(min), static_cast<int>(max));

    output.convertTo(output, CV_8UC1);
    calc_hist(output, out_hist);
    DUMP_HIST_N(dump, out_hist, static_cast<int>(optimal_threshold),"Analyzer/stretch_hist_out_5p_%d.png", pos);
    //    TRACE(dump, "* stretch_histogram(%d): hist[min] = %d hist[max] = %d\n", pos,
    //          static_cast<int>(out_hist.at<float>(static_cast<int>(min))),
    //          static_cast<int>(out_hist.at<float>(static_cast<int>(max))));

    TRACE(dump, "< stretch_histogram(%d)\n", pos);
}
//------------------------------------------------------------------------------
double find_threshold(const Mat &hist) {
    TRACE(dump, "> find_threshold(VALLEY_THRESHOLD = %d, STREAK = %d)\n",
          VALLEY_THRESHOLD, VALLEY_STREAK);

    int streak { 0 }, initial_bin { -1 };
    float val;

    for (int i = 0; i < hist.size[0]; i++) {
        val = hist.at<float>(i);

        if (val < VALLEY_THRESHOLD) {
            if (0 == streak++) {
                initial_bin = i;
                TRACE(dump, "* find_threshold(%d): start streak at %d\n", pos, i);
            }
            TRACE(dump, "* find_threshold(%d): streak = %d\n", pos, streak);
            if (streak == VALLEY_STREAK) break;
        } else {
            TRACE(dump, "* find_threshold(%d): restart streak at %d\n", pos, i);
            streak = 0;
        }
    }

    double threshold { -1.0 };
    if (streak == VALLEY_STREAK) {
        threshold = initial_bin + VALLEY_STREAK/2;
        TRACE(dump, "* find_threshold(%d): threshold found = %.2f\n", pos, threshold);
    } else {
        TRACE(dump, "* find_threshold(%d): threshold not found\n", pos);
    }

    TRACE(dump, "< find_threshold()\n", pos);
    return threshold;
}
//------------------------------------------------------------------------------
void preprocess(const Mat &input, Mat &output) {
    // To grayscale
    cvtColor(input, output, COLOR_BGR2GRAY);
    TRACE(dump, "* preprocess(%d): to grayscale\n", pos);
    DUMP(dump, output, "Analyzer/gray_%d.png", pos);

    // Contrast adjustment
    // output *= CONTRAST_FACTOR;
    // TRACE(dump, "* preprocess(%d): contrast adjusted\n", pos);

    // Stretch histogram
    // stretch_histogram(output, output);
    // DUMP(dump, output, "Analyzer/stretched_%d.png", pos);
    // TRACE(dump, "* preprocess(%d): histogram streteched\n", pos);
}
//------------------------------------------------------------------------------
#define PLOT_FACTOR 5
#define INIT_POS 0
template <typename T>
extern void plot_vector(const std::vector<T> v, cv::Mat &plot) {
    int padding = 15;
    int   width = v.size();
    int  height = static_cast<int>(0.75 * width);

    cv::Size size(width + padding * 2, height + padding * 2);
    cv::Mat img(size.height, size.width, CV_8U, cv::Scalar(255));

    cv::Rect mask(padding, padding, width, height);
    cv::Mat area = img(mask);

    T max_elem = *(max_element(v.begin(), v.end()));
    double factor = size.height / max_elem;

    for (uint i = 0; i < v.size(); i++) {
        int x = i;
        int y = static_cast<int>(v[i] * factor);

        cv::Point a(x, height);
        cv::Point b(x, height - y);

        cv::line(area, a, b, cv::Scalar(0));

        // X axis labels
        if (i % 50 == 0) {
            cv::putText(img, std::to_string(INIT_POS+i*PLOT_FACTOR), cv::Point(padding + x, padding + height + 10),
                        cv::FONT_HERSHEY_SIMPLEX, 0.3, cv::Scalar(0));
        }
    }

    // Y axis labels
    int values = 10;
    for (int i = 0; i < values; i++) {
        int value = static_cast<int>(i * max_elem / values);
        int pos = height - (i * height / values);
        std::string text = std::to_string(value);

        cv::putText(img, text, cv::Point(padding - 10, padding + pos),
                    cv::FONT_HERSHEY_SIMPLEX, 0.3, cv::Scalar(0));
    }

    plot = img;
}
//------------------------------------------------------------------------------
/*
 * Necesito hacer backtracking para saber si es de día o noche. Entonces primer
 * recorrido del video para decidir si es dia o noche, guardando resultados en
 * vector de bool. A medida que se recorre por segunda vez para calcular angulos
 * se sacan los valores del vector para volcarlos en el csv.
 *
 * Last brightness almacena el valor de brillo anterior, para cada frame calcular
 * brillo y calcular diferencia con anterior, si la dif no supera un umbral, almacenar
 * brillo en last_br y pasar al siguiente.
 * Si sí, no actualizar last_br, calcular diferencia con los siguientes frames.
 * Si la diferencia con estos tmb supera umbral (3?5? veces --> contar veces),
 * cambio. Siendo la pos de last_br la ultima del ultimo periodo (dia/noche)
 *
 * umbral diferencia ~10
 *
*/
//------------------------------------------------------------------------------
void day_or_night_test() {
    TRACE(true, "> day_or_night()\n");
    Mat f;
    // plot
    vector<double> mean_vals;
    double brightness;
    double sum { .0 };
    int num { PLOT_FACTOR };
    // alg
    double last_br, dif;
    int last_pos { 0 };
    int n { 0 };
    vector<int> breakpoints;

    // Initial brightness
    if (INIT_POS) video.set(CAP_PROP_POS_FRAMES, INIT_POS);
    video >> f;
    cvtColor(f, f, COLOR_BGR2GRAY);
    last_br = mean(f)[0];
    last_pos = INIT_POS;
    breakpoints.push_back(INIT_POS);

    for (pos = INIT_POS+1;; pos++) {
        video >> f;
        if (f.empty()) break;
        dump = pos%100==0;

        cvtColor(f, f, COLOR_BGR2GRAY);
        brightness = mean(f)[0];

        // Algorithm -----------------------------------------------------------
        if ((dif = abs(last_br - brightness)) > TOLERANCE) {
            // Do not update last_br
            n++;
            TRACE(true, "* day_or_night(): !! (%d) last_pos = %d last_br = %.2f pos = %d brightness = %.2f diference = %.2f\n",
                  n, last_pos, last_br, pos, brightness, dif);
        } else {
            TRACE(n!=0, "* day_or_night(): -- (%d) last_pos = %d last_br = %.2f pos = %d brightness = %.2f diference = %.2f\n",
                  n, last_pos, last_br, pos, brightness, dif);

            // Update last_br
            last_br = brightness;
            last_pos = pos;
            n = 0;
        }

        if (n == TIME_STREAK) {
            // Consider as day/night breakpoint
            TRACE(n!=0, "* day_or_night(): breakpoint found: %d\n", last_pos+1);
            if (abs(breakpoints[breakpoints.size() - 1] - pos)>MIN_LENGTH)
                breakpoints.push_back(last_pos+1);
            last_br = brightness;
            last_pos = pos;
            n = 0;
        }

        // Plot ----------------------------------------------------------------
        sum += brightness;
        if (pos % num == 0) {
            mean_vals.push_back(sum / num);
            sum = 0.;
        }
    }

    Mat plot;
    plot_vector(mean_vals, plot);
    cvtColor(plot, plot, COLOR_GRAY2BGR);
    for(uint i = 0; i < breakpoints.size(); i++) {
        vertical_line(plot, (breakpoints[i] - INIT_POS)/PLOT_FACTOR);
    }
    DUMP(true, plot, "Analyzer/brightness.png");

    TRACE(true, "* day_or_night(): breakpoints.size = %d\n", breakpoints.size());
    TRACE(true, "< day_or_night()\n");
}
//------------------------------------------------------------------------------
void day_or_night(const Mat &input) {
    double brightness = mean(input)[0];
    br_plot.push_back(brightness);

    if (pos == INITIAL_POS) {
        // Initialize structure
        last_br = brightness;
        last_pos = pos;
        dif_streak = 0;
        breakpoints.push_back(pos);
    } else {
        // Calculate diference in brightness since last pos
        double dif = abs(brightness - last_br);
        if (dif > TOLERANCE) {
            // Increment times tolerance surpassed
            dif_streak++;
            TRACE(true, "* day_or_night_(): !! (%d) last_pos = %d last_br = %.2f"
                        " pos = %d brightness = %.2f diference = %.2f\n",
                  dif_streak, last_pos, last_br, pos, brightness, dif);
        } else {
            TRACE(dif_streak!=0, "* day_or_night(): -- (%d) last_pos = %d last_br = %.2f"
                                 " pos = %d brightness = %.2f diference = %.2f\n",
                  dif_streak, last_pos, last_br, pos, brightness, dif);

            // Tolerance not surpassed, update last_br
            last_br = brightness;
            last_pos = pos;
            dif_streak = 0;
        }

        // Tolerance surpassed TIME_STREAK times
        if (dif_streak == TIME_STREAK) {
            TRACE(true, "* day_or_night(): TIME_STREAK: %d\n", last_pos+1);
            // If last period is long enough to be full day/night, store breakpoint
            if (abs(breakpoints[breakpoints.size() - 1] - pos) > MIN_LENGTH) {
                breakpoints.push_back(++last_pos);
                update_data(last_pos);
                TRACE(true, "* day_or_night(): ### breakpoint found: %d\n", last_pos);
            }
            // Restart to this point
            last_br = brightness;
            last_pos = pos;
            dif_streak = 0;
        }
    }
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
void otsu_threshold(const Mat &input, Mat &output) {
    TRACE(dump, "> otsu_threshold(%d)\n", pos);

    Rect roi(roi_corner, roi_sz);
    Mat otsu_in = input(roi);
    Mat otsu_out(otsu_in.size(), otsu_in.type()); // will be discarded

    TRACE(dump, "* otsu_threshold(%d): running Otsu threshold...\n", pos);
    // Get optimal threshold by applying Otsu algorithm to ROI
    double thresh = threshold(otsu_in, otsu_out, 255, 255, THRESH_OTSU);
    TRACE(dump, "* otsu_threshold(%d): thresh = %d\n", pos, static_cast<int>(thresh));

    TRACE(dump, "* otsu_threshold(%d): creating output...\n", pos);
    // Apply threshold to whole image
    threshold(input, output, thresh, 255, THRESH_BINARY);

    // Display threshold on histogram
    Mat histogram, plot;
    TRACE(dump, "* otsu_threshold(%d): calculating histogram...\n", pos);
    calc_hist(input, histogram); // mask already set
    TRACE(dump, "* otsu_threshold(%d): plotting histogram...\n", pos);
    plot_hist(histogram, plot, static_cast<int>(thresh));

    DUMP(dump, output, "Analyzer/otsu_%d.png", pos);
    DUMP(dump, plot, "Analyzer/hist_%d.png", pos);
    TRACE(dump, "< otsu_threshold(%d)\n", pos);
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
//    Rect pot(0, POT_Y_POS, output.cols, output.rows - POT_Y_POS - BAR_HEIGHT);
    Rect pot(0, POT_Y_POS - 5, output.cols, output.rows - POT_Y_POS - BAR_HEIGHT);
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
    for (i = INITIAL_POS; !found && first && i < ref.rows; i++) {
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

    // Data output
    (left ? left_leaf_vector : right_leaf_vector).push_back(ellipse_rect.angle);
    if (left)
        data_out[pos%DATA_OUT_NUM].left_angle = ellipse_rect.angle;
    else
        data_out[pos%DATA_OUT_NUM].right_angle = ellipse_rect.angle;

    TRACE(c_drawn > 2, "* ellipse_fitting(%d, %s): contours drawn = %d\n", pos,
         (left ? "left" : "right"), c_drawn);
    TRACE(dump, "< ellipse_fitting(%d, %s)\n", pos, left ? "left" : "right");
    DUMP(dump, output/*_ref*/, "Analyzer/ellipses_%d_%s.png", pos, (left ? "left" : "right"));
    DUMP(dump, contours_out, "Analyzer/contours_%d_%s.png", pos, (left ? "left" : "right"));
}
//------------------------------------------------------------------------------
void run_better_analyzer_otsu() {
    TRACE(true, "> run_better_analyzer()\n");
    initialize();
    TRACE(true, "* run_better_analyzer(): roi(%d x %d)\n",
          roi_sz.width, roi_sz.height);

    #ifdef VIDEO_OUTPUT
    init_windows();
    #endif

    int initial_pos = INITIAL_POS;
    if (initial_pos != 0) {
        TRACE(true, "* run_better_analyzer(): setting video position at %d\n", initial_pos);
        video.set(CAP_PROP_POS_FRAMES, initial_pos);
        TRACE(true, "* run_better_analyzer(): position set\n");
        pos = initial_pos;
    }

    TRACE(true, "* run_better_analyzer(): starting loop...\n");
    for (;; pos++) {
        video >> input;

        if (/*pos == 1051 ||*/ input.empty()) break;
        dump = (pos % 150 == 0 || pos == 49);

        // Preprocess image
        preprocess(input, preproc_output);

        // Thresholding
        // percentile_threshold(preproc_output, thresh_output);
        otsu_threshold(preproc_output, thresh_output);

        // Segmentation
        leaf_segmentation(thresh_output, left_leaf, right_leaf, stem);

        // Ellipse fitting
        // with Otsu thresholding, image already clean
        // close(5, left_leaf); // clean to avoid unwanted contours
        // close(5, right_leaf); // clean to avoid unwanted contours
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

        TRACE(dump, "*** run_better_analyzer: %d segundos analizados\n", pos/30);
    }

    destroyAllWindows();
    TRACE(true, "< run_better_analyzer()\n");
}
//------------------------------------------------------------------------------
void run_better_analyzer() {
    TRACE(true, "> run_better_analyzer()\n");
    initialize();
    TRACE(true, "* run_better_analyzer(): roi(%d x %d)\n",
          roi_sz.width, roi_sz.height);

    Rect roi(roi_corner, roi_sz);

    #ifdef VIDEO_OUTPUT
    init_windows();
    #endif

    // Find threshold
    #ifndef OTSU
    dump = true; pos = -1;
    video >> input;
    cvtColor(input, preproc_output, COLOR_BGR2GRAY);
    stretch_histogram(preproc_output, stretch_hist_output, stretch_output);
    optimal_threshold = find_threshold(stretch_hist_output);
    video.set(CAP_PROP_POS_FRAMES, 0);
    #endif

    int initial_pos = INITIAL_POS;
    if (initial_pos != 0) {
        TRACE(true, "* run_better_analyzer(): setting video position at %d\n", initial_pos);
        video.set(CAP_PROP_POS_FRAMES, initial_pos);
        TRACE(true, "* run_better_analyzer(): position set\n");
        pos = initial_pos;
    }

    TRACE(true, "* run_better_analyzer(): starting loop...\n");
    for (pos = 0;; pos++) {
        video >> input;

        if (input.empty()) break;
        dump = (pos % 150 == 0 || pos == 49);

        // Preprocess image
        preprocess(input, preproc_output);

        // Day or night
        day_or_night(preproc_output);

        // Stretch histogram
        stretch_histogram(preproc_output, stretch_hist_output, stretch_output);
        DUMP(dump, stretch_output, "Analyzer/stretched_5p_%d.png", pos);
        // stretch_histogram_minmax(preproc_output, stretch_output);
        // DUMP(dump, stretch_output, "Analyzer/stretched_mm_%d.png", pos);

        // Thresholding
        // percentile_threshold(preproc_output, thresh_output);
        #ifndef OTSU
        threshold(stretch_output, thresh_output, optimal_threshold, 255.0, THRESH_BINARY);
        #else
        otsu_threshold(preproc_output, thresh_output);
        #endif
        DUMP(dump, thresh_output, "Analyzer/thresh_%d.png", pos);

        // Segmentation
        leaf_segmentation(thresh_output, left_leaf, right_leaf, stem);

        // Ellipse fitting
        // close(5, left_leaf); // clean to avoid unwanted contours
        // close(5, right_leaf); // clean to avoid unwanted contours
        cvtColor(thresh_output, l_output, COLOR_GRAY2BGR);
        cvtColor(thresh_output, r_output, COLOR_GRAY2BGR);
        ellipse_fitting(left_leaf, true, l_output);
        ellipse_fitting(right_leaf, false, r_output);

        // Output video feed
        #ifdef VIDEO_OUTPUT
        imshow("thresholding", thresh_output(roi));
        imshow("stem", stem(roi));
        imshow("left_leaf", left_leaf(roi));
        imshow("right_leaf", right_leaf(roi));
        imshow("left_ellipses", l_output(roi));
        imshow("right_ellipses", r_output(roi));
        waitKey(frame_ms);
        #endif

        // Fill data
        data_out[pos%DATA_OUT_NUM].pos = pos;
        data_out[pos%DATA_OUT_NUM].night = static_cast<int>(night);
        if (pos % DATA_OUT_NUM == DATA_OUT_NUM - 1)
            dump_out_data();

        TRACE(dump, "*** run_better_analyzer: %d segundos analizados\n", pos/30);
    }

    destroyAllWindows();
    data_of.close();
    TRACE(true, "< run_better_analyzer()\n");
}
//------------------------------------------------------------------------------
// Not used --------------------------------------------------------------------
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
