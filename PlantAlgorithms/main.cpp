#include <iostream>

//#include "GlobalSegmentator.h"
//#include "PercentileSegmentator.h"
//#include "BaseAnalyzer.h"
//#include "DayNightSegmentator.h"
#include "BetterAnalyzer.h"

using namespace std;

int main()
{

// TESTS
//    run_day_night_segmentator();
//    run_base_analyzer();
//    dump_frame(2400);

    run_better_analyzer();

    cout << "Done." << endl;

    return 0;
}
