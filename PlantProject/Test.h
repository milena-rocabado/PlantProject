#ifndef TEST_H
#define TEST_H

#include <Segmentator.h>

class Test : public Segmentator
{
public:
    Test();

    void crop_test();
    void show_frames();
    void variations();
    void variations_2();
    void breakpoint_increment();

    void process_video() {}

};

#endif // TEST_H
