#include "ROIWindow.h"

#include "Traces.h"

#include <opencv2/highgui.hpp>

//------------------------------------------------------------------------------
ROIWindow::ROIWindow()
{}
//------------------------------------------------------------------------------
void ROIWindow::setImage(const cv::Mat &image) {
    image_ = image;
}//------------------------------------------------------------------------------
void ROIWindow::callback(int event, int, int, int, void *) {
    if (event == cv::EVENT_CLOSE) {
        cv::destroyWindow(WIN_NAME);
    }
}
//------------------------------------------------------------------------------
cv::Rect ROIWindow::getROI() {
    TRACE("> ROIWindow::getROI()");
    cv::Rect roi = cv::selectROI(WIN_NAME, image_);
    TRACE("* ROIWindow::getROI(): Selection done!");
    cv::namedWindow(WIN_NAME);
    cv::destroyWindow(WIN_NAME);
    TRACE("< ROIWindow::getROI()");
    return roi;
}
//------------------------------------------------------------------------------
