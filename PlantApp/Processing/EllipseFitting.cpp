#include "EllipseFitting.h"
#include "Traces.h"

//------------------------------------------------------------------------------
void EllipseFitting::close_(int size, cv::Mat &image) {
    cv::Mat element = getStructuringElement(cv::MORPH_RECT, cv::Size(size, size));
    morphologyEx(image, image, cv::MORPH_CLOSE, element);
}
//------------------------------------------------------------------------------
void EllipseFitting::drawEllipse_(cv::Mat &canvas, const cv::RotatedRect &ellipse) {
    cv::Point2f points[4];
    ellipse.points(points);

    auto middle = [] (cv::Point a, cv::Point b) -> cv::Point {
        return cv::Point((a.x + b.x)/2, (a.y + b.y)/2);
    };

    cv::line(canvas, middle(points[0], points[1]), middle(points[2], points[3]), GREEN);
    cv::line(canvas, middle(points[1], points[2]), middle(points[3], points[0]), GREEN);

    cv::ellipse(canvas, ellipse, RED);
}
//------------------------------------------------------------------------------
void EllipseFitting::fitEllipse_(const cv::Mat &input, cv::Mat &output,
                                 cv::Mat &contoursOut, float &angle) {
    assert(!roi_.empty());
    std::vector<std::vector<cv::Point>> contours;

    cv::Mat inputRef, outputRef;
    inputRef = (input(roi_));
    outputRef = output(roi_);
    contoursOut = outputRef.clone();

    // Finds contours establishing a full hierarchy of nested contours, storing
    // both endpoints of each contour
    findContours(inputRef, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
    drawContours(contoursOut, contours, -1, { 0, 0, 255 });

    size_t max { 0 }, index { 0 };

    // For each contour, find and draw ellipse if big enough
    for (size_t i = 0; i < contours.size(); i++) {
        if (contours[i].size() > max) {
            max = contours[i].size();
            index = i;
        }
    }
    cv::RotatedRect ellipseRect = cv::fitEllipse(contours[index]);
    drawEllipse_(outputRef, ellipseRect);

    // Data output
    angle = ellipseRect.angle; // eje horizontal, aumenta en sentido horario
}
//------------------------------------------------------------------------------
void EllipseFitting::process(const cv::Mat &leaf, const cv::Mat &canvas,
                             cv::Mat &ellipseDrawing,
                             float &angle, common::Side side) {
    TRACE_P(pos_, "> EllipseFitting::process(%d, %s)", pos_,
            side == common::LEFT ? "left" : "right");

    // Store copy of input
    cv::Mat leafInput = leaf.clone();

    // To BGR
    cv::cvtColor(canvas, ellipseDrawing, cv::COLOR_GRAY2BGR);

    fitEllipse_(leafInput, ellipseDrawing,
                side == common::LEFT ? contoursLOut_ : contoursROut_,
                angle);

    TRACE_P(pos_, "< EllipseFitting::process(%d, %s)", pos_,
            side == common::LEFT ? "left" : "right");

    if (side == common::RIGHT) {
        angle = 180 - angle; // cambio de eje
        pos_++;
    }
}
//------------------------------------------------------------------------------
