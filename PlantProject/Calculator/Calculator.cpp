#include "Calculator.h"

#include "Analyzer.h"
#include "qdebug.h"

using namespace std;
using namespace cv;

// ------------------------------------------------------------------
void Calculator::process_frame(const Mat &frame, Mat &dst) {
    crop_frame(frame, dst);

//    vector<vector<Point>> contours;
//    findContours(dst, contours, RETR_TREE, CHAIN_APPROX_NONE);

//    if (_first) {
//        qDebug() << "process_frame: contours size =" << contours.size();
//        for (uint i = 0, d = 0; d < 10 && i < contours.size(); i++) {
//            if (contours[i].size() > 100) {
//                qDebug() << "process_frame: contour" << i << "size ="<< contours[i].size();
//                Mat drawing = Mat::zeros(frame.size(), CV_8U);
//                drawContours(drawing, contours, static_cast<int>(i), Scalar(255, 0, 255));
//                imshow(to_string(i), drawing);
//                d++;
//            }
//        }
//        _first = false;
//        qDebug() << "process_frame: exit";
//    } else {
//        dst = Mat::zeros(frame.size(), CV_8U);
//        drawContours(dst, contours, -1, Scalar(255, 0, 255));
//    }
    find_dot(dst);
    qDebug() << "process_frame: a";
}
// ------------------------------------------------------------------
void Calculator::crop_frame(const Mat &frame, Mat &dst) {
    int height = frame.size().height - OUTPUT_HEIGHT_ADJ;
    Rect half(0, 0, frame.size().width, height);

    dst = frame.clone();
    rectangle(dst, half, Scalar(0, 0, 0), FILLED);
}
// ------------------------------------------------------------------
void Calculator::find_dot(Mat &frame) {
    // SEGMENTATORE NO INVIWERWW!!!
    cvtColor(frame, frame, COLOR_GRAY2BGR);

    int pos_x = frame.size().width / 3;
    int pos_y = OUTPUT_HEIGHT_ADJ;
    bool found = false;

    for (pos_y = 10; ! found && pos_y < frame.size().height; pos_y++) {
        for (pos_x = frame.size().width / 3; ! found && pos_x < frame.size().width * 2/3; pos_x++) {
            found = frame.at<uchar>(pos_y, pos_x) != 0;

            if (found) {
                qDebug() << "find_dot: si";
                rectangle(frame, Point(pos_x-2, pos_y-2), Point(pos_x+2, pos_y+2), Scalar(0, 0, 255), -1);
            } else {
                qDebug() << "find_dot: no";
                frame.at<Vec3b>(pos_x, pos_y) = { 0, 255, 0 };
            }
        }
    }

    //cvtColor(frame, frame, COLOR_BGR2GRAY);
}
