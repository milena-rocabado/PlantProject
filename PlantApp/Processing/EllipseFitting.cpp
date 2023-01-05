#include "EllipseFitting.h"
#include "Traces.h"
#include "Colors.h"

//------------------------------------------------------------------------------
void EllipseFitting::drawEllipse_(cv::Mat &canvas, const cv::RotatedRect &ellipse) {
    cv::Point2f points[4];
    ellipse.points(points);

    auto middle = [] (cv::Point a, cv::Point b) -> cv::Point {
        return cv::Point((a.x + b.x)/2, (a.y + b.y)/2);
    };

    cv::line(canvas, middle(points[0], points[1]), middle(points[2], points[3]), common::GREEN);
    cv::line(canvas, middle(points[1], points[2]), middle(points[3], points[0]), common::GREEN);

    cv::ellipse(canvas, ellipse, common::RED);
}
//------------------------------------------------------------------------------
void EllipseFitting::findLeafContour_(const std::vector<std::vector<cv::Point>> &contours,
                                      std::vector<cv::Point> &leafContour,
                                      cv::Mat &contoursOut) {
    TRACE_P(pos_, "> EllipseFitting::findLeafContour_(%d, %s)", pos_,
          side_ == common::LEFT ? "left" : "right");

    // For contours corresponding to leaf, store bounding rect and index
    std::vector<std::pair<int, cv::Rect>> rects(1);
    // Floor Y position
    int floorY { roi_.height };
    // Top contour lower corner Y position
    int topY { 0 };

    bool (*better)(cv::Rect a, cv::Rect b) = (side_ == common::LEFT)
            ? [] (cv::Rect a, cv::Rect b) -> bool { return a.x < b.x; } // pensar: <= ?
            : [] (cv::Rect a, cv::Rect b) -> bool { return a.x + a.width > b.x + b.width; };

    // For each contour
    for (uint contourIdx = 0; contourIdx < contours.size(); contourIdx++) {

        // Get bounding rectangle
        cv::Rect bRect = cv::boundingRect(contours[contourIdx]);

        if (bRect.y == 0 || bRect.y < topY) { // CASE A: touching top, discard
            TRACE_P(pos_, "* EllipseFitting::findLeafContour_(): discarded top contour (%d, %d) [%d x %d]",
                    bRect.x, bRect.y, bRect.width, bRect.height);
            cv::rectangle(contoursOut, bRect, common::YELLOW);
            cv::putText(contoursOut, "A",
                        cv::Point(bRect.x, bRect.y + 10),
                        cv::FONT_HERSHEY_SIMPLEX, 0.3, common::YELLOW);

            if (bRect.height > topY) {
                topY = bRect.height;
                TRACE_P(pos_, " \tupdated top = %d", topY);
            }
            continue;
        } else if (bRect.y + bRect.height >= roi_.height) { // CASE B: touching bottom, discard
            TRACE_P(pos_, "* EllipseFitting::findLeafContour_(): discarded bottom contour (%d, %d) [%d x %d]",
                    bRect.x, bRect.y, bRect.width, bRect.height);
            cv::rectangle(contoursOut, bRect, common::YELLOW);
            cv::putText(contoursOut, "B",
                        cv::Point(bRect.x, bRect.y),
                        cv::FONT_HERSHEY_SIMPLEX, 0.3, common::YELLOW);

            // Store floor Y position (if it's higher in image -> lower value)
            if (bRect.y < floorY) {
                floorY = bRect.y;
                TRACE_P(pos_, " \tupdated floor = %d", floorY);
            }
            continue;
        } else if (floorY <= bRect.y) { // CASE C: soil contour, discard
            TRACE_P(pos_, "* EllipseFitting::findLeafContour_(): discarded soil contour (%d, %d) [%d x %d]",
                    bRect.x, bRect.y, bRect.width, bRect.height);
            cv::rectangle(contoursOut, bRect, common::YELLOW);
            cv::putText(contoursOut, "C",
                        cv::Point(bRect.x, bRect.y),
                        cv::FONT_HERSHEY_SIMPLEX, 0.3, common::YELLOW);
            continue;
        }

        TRACE_P(pos_, "* EllipseFitting::findLeafContour_(): ordered insert,"
                      "collection size = %d", rects.size());
        bool inserted = false;
        for (uint rectIdx = 0; !inserted && rectIdx < rects.size(); ++rectIdx) {
            inserted = true;

            if (rects[rectIdx].second.empty()) {
                TRACE_P(pos_, "* EllipseFitting::findLeafContour_(): first");
                rects[rectIdx].first = contourIdx;
                rects[rectIdx].second = bRect;
            }
            else {
                TRACE_P(pos_, "* EllipseFitting::findLeafContour_(): Comparing"
                              "(%d, %d) [%d x %d] and (%d, %d) [%d x %d]",
                        bRect.x, bRect.y, bRect.width, bRect.height,
                        rects[rectIdx].second.x, rects[rectIdx].second.y,
                        rects[rectIdx].second.width, rects[rectIdx].second.height);

                if (better(bRect, rects[rectIdx].second)) {
                    rects.insert(rects.begin() + rectIdx, std::make_pair(contourIdx, bRect));

                    TRACE_P(pos_, "* EllipseFitting::findLeafContour_(): inserted at index %d", rectIdx);

                    for (uint k = rectIdx + 1; k < rects.size();) {
                        TRACE_P(pos_, "\tchecking rect: (%d, %d) [%d x %d]",
                                rects[rectIdx].second.x, rects[rectIdx].second.y,
                                rects[rectIdx].second.width, rects[rectIdx].second.height);

                        // If current upper corner higher rect bottom, discard
                        if (bRect.y < rects[k].second.y) { // CASE E
                            // bRect.y < rects[k].second.y
                            // bRect.y < rects[k].second.y + rects[k].second.height
                            TRACE_P(pos_, "\t--> erased");
                            cv::rectangle(contoursOut, rects[k].second, common::YELLOW);
                            cv::putText(contoursOut, "E",
                                        cv::Point(rects[k].second.x, rects[k].second.y),
                                        cv::FONT_HERSHEY_SIMPLEX, 0.3, common::YELLOW);
                            rects.erase(rects.begin() + k);
                        } else {
                            TRACE_P(pos_, "\t--> not erased");
                            k++;
                        }
                    }
                }
                else {
                    TRACE_P(pos_, "* EllipseFitting::findLeafContour_(): not inserted at index %d", rectIdx);
                    inserted = false;
                }
            }
        }
        if (!inserted) {
            TRACE_P(pos_, "* EllipseFitting::findLeafContour_(): end. attempt to push back");

            if (bRect.y <= rects.back().second.y) {
            // bRect.y + bRect.height <= rects.back().second.y
                TRACE_P(pos_, "\tinserted at the end");
                rects.push_back(std::make_pair(contourIdx, bRect));
            } else { // CASE D
                TRACE_P(pos_, "\tdiscarded error contour (%d, %d) [%d x %d]",
                        bRect.x, bRect.y, bRect.width, bRect.height);
                cv::rectangle(contoursOut, bRect, common::YELLOW);
                cv::putText(contoursOut, "D",
                            cv::Point(bRect.x, bRect.y),
                            cv::FONT_HERSHEY_SIMPLEX, 0.3, common::YELLOW);
            }
        }
    }

    TRACE_P(pos_, "* EllipseFitting::findLeafContour_(): last contour check");
    int total = 0;
    for (uint j = 0; j < rects.size(); ++j) {
        if (rects[j].second.empty())
            continue;
        if (rects[j].second.y < floorY) {
            TRACE_P(pos_, "\t* valid contour (%d, %d) [%d x %d]",
                    rects[j].second.x, rects[j].second.y,
                    rects[j].second.width, rects[j].second.height);

            leafContour.insert(leafContour.end(), contours[rects[j].first].begin(),
                    contours[rects[j].first].end());

            cv::rectangle(contoursOut, rects[j].second, common::GREEN);
            cv::putText(contoursOut, std::to_string(total),
                        cv::Point(rects[j].second.x, rects[j].second.y),
                        cv::FONT_HERSHEY_SIMPLEX, 0.3, common::MAGENTA);
            total++;
        } else { // CASE F
            TRACE_P(pos_, "\tx discarded soil contour (%d, %d) [%d x %d]",
                    rects[j].second.x, rects[j].second.y,
                    rects[j].second.width, rects[j].second.height);
            cv::rectangle(contoursOut, rects[j].second, common::YELLOW);
            cv::putText(contoursOut, "F",
                        cv::Point(rects[j].second.x, rects[j].second.y),
                        cv::FONT_HERSHEY_SIMPLEX, 0.3, common::YELLOW);

        }

    }

    TRACE_P(pos_, "< EllipseFitting::findLeafContour_(%d, %s)", pos_,
          side_ == common::LEFT ? "left" : "right");
}
//------------------------------------------------------------------------------
void EllipseFitting::fitEllipse_(const cv::Mat &input, cv::Mat &output,
                                 cv::Mat &contoursOut, float &angle) {
    assert(!roi_.empty());
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Point> leafContour;

    cv::Mat inputRef, outputRef;
    inputRef = (input(roi_));
    outputRef = output(roi_);
    contoursOut = outputRef.clone();

    // Finds contours establishing a full hierarchy of nested contours, storing
    // both endpoints of each contour
    findContours(inputRef, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
    drawContours(contoursOut, contours, -1, common::RED);
    findLeafContour_(contours, leafContour, contoursOut);

    if (leafContour.size() < 5) {
        TRACE_ERR("! EllipseFitting::fitEllipse_(%d, %s): not enough points in contour", pos_,
                  side_ == common::LEFT ? "left" : "right");
        return;
    }
    cv::RotatedRect ellipseRect = cv::fitEllipse(leafContour);
    drawEllipse_(outputRef, ellipseRect);

    // Data output
    angle = side_ == common::LEFT ? ellipseRect.angle : 180 - ellipseRect.angle;
}
//------------------------------------------------------------------------------
void EllipseFitting::process(const cv::Mat &leaf, const cv::Mat &canvas,
                             cv::Mat &ellipseDrawing,
                             float &angle, common::Side side) {
    TRACE_P(pos_, "> EllipseFitting::process(%d, %s)", pos_,
            side == common::LEFT ? "left" : "right");

    // Store current side
    side_ = side;

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
//        //_DELETE___
//        if (pos_ > 2150 && pos_ <= 2250){
//            DUMP(contoursROut_, wd_, "contours_r_%d.png", pos_);
//        } //___
//        DUMP_P(pos_, contoursROut_, wd_, "contours_r_%d.png", pos_);
        pos_++;
    } else {
//        //_DELETE___
//        if (pos_ >= 2150 && pos_ <= 2250) {
//            DUMP(contoursLOut_, wd_, "contours_l_%d.png", pos_);
//        }// ____
//        DUMP_P(pos_, contoursLOut_, wd_, "contours_l_%d.png", pos_);
    }

}
//------------------------------------------------------------------------------
