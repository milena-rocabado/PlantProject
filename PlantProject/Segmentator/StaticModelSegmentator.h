#ifndef ANALIZADORVIDEO_H
#define ANALIZADORVIDEO_H

#include <Segmentator.h>

class StaticModelSegmentator : public Segmentator
{
public:
    enum Model {
        Modelo1,
        Modelo2
    };

    typedef struct {
        const char *name;
        Model model;
        double alpha;
        double thresh;
    } Configuration;

    static constexpr Configuration CONF1 = { "Modelo global", Modelo1, 4., 80. };
    static constexpr Configuration CONF2 = { "Modelo noche", Modelo2, 1., 0. };

    StaticModelSegmentator(const Configuration &configuracion):
        _conf(configuracion)
    {}

    StaticModelSegmentator():
        _conf(CONF2)
    {}

    StaticModelSegmentator(const double &alpha, const double &thresh, const Model &modelo):
        _conf({ "", modelo, alpha, thresh })
    {}

    double alpha() { return _conf.alpha; }
    double thresh() { return _conf.thresh; }

    void set_conf(const Configuration &conf) { _conf = conf; }

    void set_up() override { load_model(); }
    void process_debug() override { process_2_frames(); }

    static std::string model_to_str(const Model &modelo) {
        switch (modelo) {
        case Modelo1: return "Modelo global";
        case Modelo2: return "Modelo noche";
        }
        return "";
    }

    static std::string conf_to_str(Configuration conf) {
        std::stringstream ss;
        ss << conf.name << ": { modelo: " << model_to_str(conf.model)
           << ", thresh: " << conf.thresh
           << ", alpha: " << conf.alpha << "}";
        return ss.str();
    }

    ~ StaticModelSegmentator() override;

protected:
    void process_frame() override;
    void show_frame(const cv::Mat &frame, const std::string &name) override;

private:
    Configuration _conf;
    cv::Mat _model;

    bool _apply_contrast;

    void process_2_frames();

    void create_mean_image(std::string fp);
    void load_model();

    void iterate_alpha();
    void iterate_blur();
    void iterate_thresh();
};

#endif // ANALIZADORVIDEO_H
