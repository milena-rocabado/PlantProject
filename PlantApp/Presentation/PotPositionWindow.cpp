#include "PotPositionWindow.h"

#include "Traces.h"
#include "Colors.h"

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

PotPositionWindow::PotPositionWindow()
{}
//------------------------------------------------------------------------------
void PotPositionWindow::setImage(const cv::Mat &image) {
    image_ = image;
    canvas_ = image_.clone();
}
//------------------------------------------------------------------------------
void PotPositionWindow::onMouse_(int event, int, int y, int, void* userdata) {

    int *posA = reinterpret_cast<int*>(userdata);
//    int *posB = reinterpret_cast<int*>(&userdata);
//    int posC = *(reinterpret_cast<int*>(userdata));
//    int posD = *(reinterpret_cast<int*>(&userdata));

    switch (event) {
    case cv::EVENT_LBUTTONDOWN:
        // Store position somehow
        TRACE("* PotPositionWindow::onMouse_(y=%d)", y);
        *posA = y;
        TRACE("* PotPositionWindow::onMouse_(posA=%d)", posA);
//        *posB = y;
//        TRACE("* PotPositionWindow::onMouse_(posB=%d)", posB);
//        posC  = y;
//        TRACE("* PotPositionWindow::onMouse_(posC=%d)", posC);
//        posD  = y;
//        TRACE("* PotPositionWindow::onMouse_(posD=%d)", posD);
        break;
    default:
        // do nothing
        break;
    }
}
//------------------------------------------------------------------------------
int PotPositionWindow::getPotPosition() {
    TRACE("> PotPositionWindow::getPotPosition()");

    bool stop = false;

    int position {-1}, lastPos {-1};

    cv::namedWindow(WIN_NAME);
    cv::setMouseCallback(WIN_NAME, PotPositionWindow::onMouse_, &position);

    while (!stop) {

        if (position != lastPos) {
            TRACE("* PotPositionWindow::getPotPosition(): position = %d", position);
            canvas_ = image_.clone();
            cv::line(canvas_, cv::Point(0, position),
                     cv::Point(canvas_.cols, position), common::GREEN, 2);
            lastPos = position;
        }
        cv::imshow(WIN_NAME, canvas_);

        int c = cv::waitKey(30);
        switch (c) {
        case 'c':
            // Clear
            canvas_ = image_.clone();
            break;
        case ' ':
            // Exit
            stop = true;
            break;
        }
    }

    cv::destroyWindow(WIN_NAME);
    TRACE("< PotPositionWindow::getPotPosition() -> %d", position);
    return position;
}
