#include "ROIWindow.h"

#include "AnalyzerManager.h"
#include "Traces.h"

//------------------------------------------------------------------------------
ROIWindow::ROIWindow()
{}
//------------------------------------------------------------------------------
void ROIWindow::setImage(const cv::Mat &image) {
    image_ = image;
}
//------------------------------------------------------------------------------
cv::Rect ROIWindow::getROI() {
    TRACE(">ROIWindow::getROI()");
    cv::Rect roi = cv::selectROI(WIN_NAME, image_);
    TRACE("<ROIWindow::getROI(): Selection done!");
    cv::destroyWindow(WIN_NAME);
    TRACE("<ROIWindow::getROI()");
    return roi;
}
//------------------------------------------------------------------------------
