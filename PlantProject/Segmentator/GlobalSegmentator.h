#ifndef ANALIZADORFRAMES_H
#define ANALIZADORFRAMES_H

#include "Segmentator.h"
#include <QMetaType>
#include <map>

class GlobalSegmentator : public Segmentator
{
public:

    enum ThreshType {
        Medio,
        Fijo,
        AdaptativoMedia,
        AdaptativoGauss,
        Otsu,
        ThreshTypeNum
    };

    typedef struct {
        ThreshType thresh_type;
        double alpha;
    } Configuracion;

    static constexpr double ALPHA_DF { 2.0 };

    static constexpr Configuracion CONF_DF { Fijo, ALPHA_DF };

    GlobalSegmentator():
        _conf(CONF_DF)
    {}

    GlobalSegmentator(const ThreshType &type, double alpha = ALPHA_DF):
        _conf({type, alpha})
    {}

    ThreshType thresh_type() const;
    double alpha() const;

    void set_up() override {}
    void process_video();
    void process_debug() override { process_2_frames(); }

    static std::string umbralizado_to_str(ThreshType u);

    // ONE TIME FUNCTIONS
    void calculate_average_middle_point();
    void calculate_percentile_thresh();

    // TEST FUNCTIONS
    void process_2_frames();
    void process_2_frames(cv::Mat &night, cv::Mat &day);

protected:
    void process_frame() override;
    void show_frame(const cv::Mat &frame, const std::string &name) override;

private:
    Configuracion _conf;
    cv::Rect _roi; // crop frame before thresholding

    // PREPROCESADO
    void preprocess();

    // UMBRALIZADO
    void umbralizar();

    static const std::map<ThreshType, double (*)(const cv::Mat&, cv::Mat&)> umbrs;

    static double umbral_medio(const cv::Mat &src, cv::Mat &dst);
    static double umbral_fijo(const cv::Mat &src, cv::Mat &dst);
    static double umbral_adaptativo_media(const cv::Mat &src, cv::Mat &dst);
    static double umbral_adaptativo_gauss(const cv::Mat &src, cv::Mat &dst);
    static double umbral_otsu(const cv::Mat &src, cv::Mat &dst);
};

#endif // ANALIZADORFRAMES_H
