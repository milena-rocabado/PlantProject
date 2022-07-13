#ifndef TEST_H
#define TEST_H

#include <Segmentator.h>

class TestSegmentator : public Segmentator
{
public:
    TestSegmentator();

    void crop_test();
    void show_frames();
    void variations();
    void variations_2();
    void breakpoint_increment();

    void process_video() {}

};

void test_outside_loop();
void test_analyzer();

#endif // TEST_H
