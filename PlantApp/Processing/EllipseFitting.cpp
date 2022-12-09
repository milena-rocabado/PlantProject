#include "EllipseFitting.h"
#include "Traces.h"

//------------------------------------------------------------------------------
void EllipseFitting::close_(int size, cv::Mat &image) {
    cv::Mat element = getStructuringElement(cv::MORPH_RECT, cv::Size(size, size));
    morphologyEx(image, image, cv::MORPH_CLOSE, element);
}
//------------------------------------------------------------------------------
void EllipseFitting::fitEllipse_(const cv::Mat &input, cv::Mat &output,
                                 float &angle) {
    assert(!roi_.empty());
    std::vector<std::vector<cv::Point>> contours;

    cv::Mat inputRef, outputRef;
    inputRef = input(roi_);
    outputRef = output(roi_);
    contoursOut_ = outputRef.clone();

    // Finds contours establishing a full hierarchy of nested contours, storing
    // both endpoints of each contour
    findContours(inputRef, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
    drawContours(contoursOut_, contours, -1, { 0, 0, 255 });

    size_t max { 0 }, index { 0 };

    // For each contour, find and draw ellipse if big enough
    for (size_t i = 0; i < contours.size(); i++) {
        if (contours[i].size() > max) { // contours[i].size() > CONTOUR_MIN_SIZE
            max = contours[i].size();
            index = i;
        }
    }
    cv::RotatedRect ellipseRect = cv::fitEllipse(contours[index]);
    cv::ellipse(outputRef, ellipseRect, RED);

    // Data output
    angle = ellipseRect.angle;
}
//------------------------------------------------------------------------------
void EllipseFitting::process(const cv::Mat &leaf, const cv::Mat &canvas,
                             cv::Mat ellipseDrawing,
                             float &angle, common::Side side) {
    TRACE_P(pos_, "> EllipseFitting::process(%d, %s)\n", pos_,
            side == common::LEFT ? "left" : "right");

    // Store copy of input
    cv::Mat leafInput = leaf.clone();
    // Clean up noise leftover from thresholding and leaf segmentation
    close_(5, leafInput);

    // To BGR
    cv::cvtColor(canvas, ellipseDrawing, cv::COLOR_GRAY2BGR);

    fitEllipse_(leafInput, ellipseDrawing, angle);

    TRACE_P(pos_, "< EllipseFitting::process(%d, %s)\n", pos_,
            side == common::LEFT ? "left" : "right");
    pos_++;
}
