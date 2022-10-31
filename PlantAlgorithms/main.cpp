#include <iostream>

//#include "GlobalSegmentator.h"
//#include "PercentileSegmentator.h"
//#include "BaseAnalyzer.h"
//#include "DayNightSegmentator.h"
#include "BetterAnalyzer.h"

using namespace std;

int main()
{
// >> FUTURE USE: VIDEO PLAYER <<
//    // play video
//    cv::VideoCapture video(string("C:/Users/radsa/git/TFG-PlantProject/Media/") + "climbing_bean_project3_leaf_folding.AVI");
//    if (! video.isOpened()) {cout << "oops" << endl; return -1; }
//    cv::Mat frame;
//    cv::namedWindow("video");
//    for (;;) {
//        video >> frame;
//        if (frame.empty()) break;

//        cv::imshow("video", frame);
//        cout << "doint it !!" << endl;

//        if (cv::waitKey(33) == 27)
//        {
//          break;
//        }
//    }
//    cout << "over." << endl;

// TESTS
//    run_day_night_segmentator();
//    run_base_analyzer();
    run_better_analyzer();

    cout << "Done." << endl;

    return 0;
}
