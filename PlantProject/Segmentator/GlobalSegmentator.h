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
        Otsu
    };

    static constexpr double ALPHA_DF = 2.0;
    static constexpr ThreshType THRESHTYPE_DF = ThreshType::Fijo;

    GlobalSegmentator():
        _threshType(THRESHTYPE_DF),
        _alpha(ALPHA_DF)
    {}

    bool set_video(const std::string& path) override;
    void set_up() override {}
    void process_video();
    void process_debug() override { process_2_frames(); }

    ThreshType umbralizado();
    double contraste();

    void set_threshType(const ThreshType &);
    void set_alpha(const double &);

    static std::string umbralizado_to_str(ThreshType u) {
        switch (u) {
        case Medio:           return "Medio";
        case Fijo:            return "Fijo";
        case AdaptativoMedia: return "Adaptativo de media";
        case AdaptativoGauss: return "Adaptativo gaussiano";
        case Otsu:            return "Binarización de Otsu";
        }
        return "Invalid enum";
    }

private:
    ThreshType _threshType;
    double _alpha;

    void process_2_frames();
    void process_frame() override;
    void show_frame(const cv::Mat &frame, const std::string &name) override;

    // PREPROCESADO
    void preprocesar();

    // UMBRALIZADO
    void umbralizar();

    static const std::map<ThreshType, double (*)(const cv::Mat&, cv::Mat&)> umbrs;

    static double umbral_medio(const cv::Mat &src, cv::Mat &dst);
    static double umbral_fijo(const cv::Mat &src, cv::Mat &dst);
    static double umbral_adaptativo_media(const cv::Mat &src, cv::Mat &dst);
    static double umbral_adaptativo_gauss(const cv::Mat &src, cv::Mat &dst);
    static double umbral_otsu(const cv::Mat &src, cv::Mat &dst);

    // MORFOLOGÍA
    void invertir();
    void abrir();
};

#endif // ANALIZADORFRAMES_H
