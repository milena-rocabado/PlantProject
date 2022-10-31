#include "DayNightSegmentator.h"

#include "common.h"
#include "GlobalSegmentator.h"

#include <string>

using namespace std;
using namespace cv;

extern string wd;

static int pos;
static bool dump;

/*
 * Para determinar si día o noche: luminosidad de la imagen cercana a 110 o no.
 * Almacenar media de luminosidad de imágenes desde ultimo breakpoint, comparar
 * luminosidad actual con luminosidad media. Si difiere “mucho”, cambio.
 * Reiniciar media.
 *
 * BREAKPOINT BRIGHTNESS PERCENTILE
 *        436        101   0.195128
 *       1395        109   0.196291
 *       1877        100   0.200755
 *       2836        108   0.207291
 *       3314         84   0.220317
 *       4188        107   0.218791
 *
 * Percentil calculado aplicando un umbral fijo independiente de variable dia/
 * noche.
 *
 * Recalcular umbral en funcion de dia/noche (¿mitad de la media de luminosidad
 * anterior ?)
 */

void dn_preprocess(const Mat &src, Mat &dst) {

    // Recortar
    crop_roi(src, dst);
    if (dump)
        save("DayNight/" + to_string(pos) + "_cropped", dst);

    // A escala de grises
    cvtColor(dst, dst, COLOR_RGB2GRAY);
    if (dump)
        save("DayNight/" + to_string(pos) + "_gs", dst);
}
//------------------------------------------------------------------------------
void plot_percentile(Histogram calc_hist, float percentile, Mat img, Mat &plot) {
    Mat hist;
    float value = static_cast<float>(img.cols * img.rows * percentile);

    calc_hist(img, hist);

    float sum { 0.0f };
    int position { 0 };
    for (; position < HIST_SIZE; position++) {
        sum += hist.at<float>(position);
        if (sum >= value) break;
    }

    plot_hist(hist, plot);
    cvtColor(plot, plot, COLOR_GRAY2BGR);
    vertical_line(plot, position);
}
//------------------------------------------------------------------------------
void run_day_night_segmentator() {
    VideoCapture video(wd + "climbing_bean_project3_leaf_folding.AVI");

    int  out_width = static_cast<int>(video.get(CAP_PROP_FRAME_WIDTH) / 3.0);
    int out_height = static_cast<int>(video.get(CAP_PROP_FRAME_HEIGHT)) - BAR_HEIGHT;
    VideoWriter salida(wd + "/dn_dump_out.AVI", static_cast<int>(video.get(CAP_PROP_FOURCC)),
                       video.get(CAP_PROP_FPS),
                       Size(out_width, out_height), false);
    Size plot_sz;
    plot_size(HIST_SIZE, plot_sz);

    VideoWriter plot_out(wd + "/plot_out.AVI", static_cast<int>(video.get(CAP_PROP_FOURCC)),
                       video.get(CAP_PROP_FPS),
                       plot_sz, true);

    Mat frame, preprocessed, output;

    // for percentile calculation
    Mat hist_binary;
    Histogram calc_hist_binary;
    calc_hist_binary.set_hist_size(2);

    // for plotting percentile
    Histogram calc_hist;
    Mat hist, plot;

    int num_samples { 1 };
    int sum_brightness { 0 };
    float sum_percentile { 0 };

    int brightness { 0 };
    double thresh { 0. };
    float percentile { 0.f };

    int bp;
    for (pos = 0u, bp = 0; ; pos++, dump = (pos == NIGHT_SAMPLE || pos == DAY_SAMPLE), num_samples++ ) {
        video >> frame;

        if (frame.empty()) break;
        if (pos % 150 == 0)
            cout << "run_day_night_segmentator: analizados " + to_string(pos/30) + " segundos" << endl;

        // preprocess
        dn_preprocess(frame, preprocessed);

        // calculate brightness
        brightness = static_cast<int>(mean(preprocessed)[0]);
        sum_brightness += brightness;

        // threshold using half average brightness
        thresh = 0.65 * (sum_brightness / num_samples); // mitad de media
        threshold(preprocessed, output, thresh, 255, THRESH_BINARY);

        salida << output;

        // calculate percentile
        calc_hist_binary(output, hist_binary);
        percentile = hist_binary.at<float>(0) / (output.cols * output.rows);
        sum_percentile += percentile;

        plot_percentile(calc_hist, percentile, preprocessed, plot);
        if (pos == 1010) {cout << "idk" << endl; save("idk", plot);}
        plot_out << plot;

        if (pos == DN_BREAKPOINTS[bp]) {
            cout << "run_day_night_segmentator: breakpoint at " << pos
                 << "(num_samples = " << num_samples << ", p_sum = " << sum_percentile
                 << ", b_sum = " << sum_brightness << ")" << endl;

            sum_percentile = 0.f; sum_brightness = 0; num_samples = 0;
            bp++;
        }
    }
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
 */
