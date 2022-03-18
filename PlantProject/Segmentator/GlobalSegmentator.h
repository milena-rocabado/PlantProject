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

    typedef struct {
        ThreshType thresh_type;
        double alpha;
    } Configuracion;

    static constexpr Configuracion CONF_DF = { Fijo, 2.0 };

    GlobalSegmentator():
        _conf(CONF_DF)
    {}

    GlobalSegmentator(const ThreshType &type, const double &alpha):
        _conf({type, alpha})
    {}

    ThreshType thresh_type() const;
    double alpha() const;

    bool set_video(const std::string& path) override;
    void set_up() override {}
    void process_video();
    void process_debug() override { process_2_frames(); }

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
protected:
    void process_frame() override;
    void show_frame(const cv::Mat &frame, const std::string &name) override;

private:
    Configuracion _conf;

    void process_2_frames();

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
};

#endif // ANALIZADORFRAMES_H
