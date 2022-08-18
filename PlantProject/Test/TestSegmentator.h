#ifndef TEST_H
#define TEST_H

#include <Segmentator.h>

class TestSegmentator : public Segmentator
{
public:
    TestSegmentator();

    void crop_test();

    void mean_values();

    void variations();
    void variations_2();
    void breakpoint_surrounding_mean_values();

    void dump_histograms();

    //---------------------------------------------------
    void process_debug() {}
    void process_frame() {}
    void set_up() {}
};

void determine_hist_percentile();

void test_outside_loop();
void test_analyzer();

#endif // TEST_H
