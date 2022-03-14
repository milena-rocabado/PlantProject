#ifndef ANALIZADORSEGMENTACIONADAPTATIVO_H
#define ANALIZADORSEGMENTACIONADAPTATIVO_H

#include <Segmentator.h>
#include <fstream>
#include <vector>

class AdaptiveModelSegmentator : public Segmentator
{
public:
    static constexpr uint DN_BREAKPOINTS[] = { 436, 1395, 1876, 2836, 3314 };
    static constexpr double ALPHA_DF = 4.0;

    AdaptiveModelSegmentator()
        : _alpha(1.0)
    {}

    double alpha();
    void set_alpha(const double &);

    void process_debug() override { process_2_frames(); }
    ~AdaptiveModelSegmentator() override;

protected:
    void set_up() override;
    void process_frame() override;

private:
    cv::Mat _night_model;
    cv::Mat _day_model;
    bool _night;
    double _alpha;

    bool _apply_contrast;
    std::ofstream _file;

    void load_model(const bool &);
    void create_model();
    void process_2_frames();
    void iterarate_thresh();
};

#endif // ANALIZADORSEGMENTACIONADAPTATIVO_H
