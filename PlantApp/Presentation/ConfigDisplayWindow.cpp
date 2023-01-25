#include "ConfigDisplayWindow.h"

#include "Traces.h"

//------------------------------------------------------------------------------
const cv::Scalar ConfigDisplayWindow::ROI_COLOR { 255, 0, 0 };
const cv::Scalar ConfigDisplayWindow::POT_COLOR { 0, 255, 0 };
//------------------------------------------------------------------------------
ConfigDisplayWindow::ConfigDisplayWindow()
{}
//------------------------------------------------------------------------------
void ConfigDisplayWindow::callback(int event, int, int, int, void *) {
    if (event == cv::EVENT_CLOSE) {
        cv::destroyWindow(WIN_NAME);
    }
}
//------------------------------------------------------------------------------
void ConfigDisplayWindow::setImage(const cv::Mat &image) {
    image_ = image.clone();
}
//------------------------------------------------------------------------------
void ConfigDisplayWindow::display(const cv::Rect &roi, int yPot) {
    TRACE("> ConfigDisplayWindow::display(roi = (%d x %d), yPot = %d)",
          roi.width, roi.height, yPot);
    TRACE("* ConfigDisplayWindow::display(): image_.size = (%d x %d)",
          image_.cols, image_.rows);

    cv::Mat canvas = image_.clone();

    TRACE("* ConfigDisplayWindow::display(): canvas.size = (%d x %d)",
          canvas.cols, canvas.rows);

    // ROI with crosshair
    cv::rectangle(canvas, roi, ROI_COLOR, THICKNESS);
    cv::line(canvas,
             cv::Point(roi.x + roi.width/2, roi.y),
             cv::Point(roi.x + roi.width/2, roi.y + roi.height),
             ROI_COLOR, THICKNESS);
    cv::line(canvas,
             cv::Point(roi.x, roi.y + roi.height / 2),
             cv::Point(roi.x + roi.width, roi.y + roi.height / 2),
             ROI_COLOR, THICKNESS);

    // Pot position
    cv::line(canvas, cv::Point(0, yPot), cv::Point(canvas.cols, yPot),
             POT_COLOR, THICKNESS);

    // Show drawing
    cv::imshow(WIN_NAME, canvas);
    // Wait until key pressed
    cv::waitKey();
    // Close window
    cv::namedWindow(WIN_NAME);
    cv::destroyWindow(WIN_NAME);

    TRACE("< ConfigDisplayWindow::display()");
}
//------------------------------------------------------------------------------
