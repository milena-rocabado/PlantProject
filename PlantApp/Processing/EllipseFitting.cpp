#include "EllipseFitting.h"
#include "Traces.h"
#include "Colors.h"

//------------------------------------------------------------------------------
EllipseFitting::EllipseFitting()
    : lastLAngle_(-1.f)
    , lastRAngle_(-1.f)
    , dump_(false)
{}
//------------------------------------------------------------------------------
void EllipseFitting::initialize() {
    lastLAngle_ = -1.f;
    lastRAngle_ = -1.f;
    dump_ = false;
}
//------------------------------------------------------------------------------
void EllipseFitting::drawEllipse_(const cv::RotatedRect &ellipse, cv::Mat &canvas) {
    cv::Point2f points[4];
    ellipse.points(points);

    auto middle = [] (cv::Point a, cv::Point b) -> cv::Point {
        return cv::Point((a.x + b.x)/2, (a.y + b.y)/2);
    };

    cv::line(canvas, middle(points[0], points[1]), middle(points[2], points[3]), common::GREEN, 2);
    cv::line(canvas, middle(points[1], points[2]), middle(points[3], points[0]), common::GREEN, 2);

//    if (side_ == common::RIGHT) {
//        cv::Point a = middle(points[3], points[0]);
//        cv::line(canvas, a, cv::Point(a.x, a.y + static_cast<int>(ellipse.size.height)), common::BLUE);
//        cv::putText(canvas, std::to_string(ellipse.angle), points[0], cv::FONT_HERSHEY_SIMPLEX, 0.3, common::MAGENTA);
//    } else {
//        cv::Point a = middle(points[1], points[2]);
//        cv::line(canvas, a, cv::Point(a.x, a.y + static_cast<int>(ellipse.size.height)), common::BLUE);
//        cv::putText(canvas, std::to_string(ellipse.angle), points[1], cv::FONT_HERSHEY_SIMPLEX, 0.3, common::MAGENTA);
//    }

    cv::ellipse(canvas, ellipse, common::RED, 2);
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
    int topY { 5 };

    bool (*better)(cv::Rect a, cv::Rect b) = (side_ == common::LEFT)
            ? [] (cv::Rect a, cv::Rect b) -> bool { return a.x < b.x; }
            : [] (cv::Rect a, cv::Rect b) -> bool { return a.x + a.width > b.x + b.width; };

    // For each contour
    for (uint contourIdx = 0; contourIdx < contours.size(); contourIdx++) {

        // Get bounding rectangle
        cv::Rect bRect = cv::boundingRect(contours[contourIdx]);

        // CASE A: touching top, discard
        if (bRect.y == 0 || bRect.y < topY) {
            TRACE_P(pos_, "* EllipseFitting::findLeafContour_(): discarded top contour (%d, %d) [%d x %d]",
                    bRect.x, bRect.y, bRect.width, bRect.height);
            cv::rectangle(contoursOut, bRect, common::MAGENTA);
//            cv::putText(contoursOut, "A",
//                        cv::Point(bRect.x, bRect.y + 10),
//                        cv::FONT_HERSHEY_SIMPLEX, 0.3, common::YELLOW);

            if (bRect.height > topY) {
                topY = bRect.height;
                TRACE_P(pos_, " \tupdated top = %d", topY);
            }
            continue;
        }
        // CASE B: touching bottom, discard
        else if (bRect.y + bRect.height >= roi_.height || floorY <= bRect.y) {
            TRACE_P(pos_, "* EllipseFitting::findLeafContour_(): discarded bottom contour (%d, %d) [%d x %d]",
                    bRect.x, bRect.y, bRect.width, bRect.height);
            cv::rectangle(contoursOut, bRect, common::MAGENTA);
//            cv::putText(contoursOut, "B",
//                        cv::Point(bRect.x, bRect.y),
//                        cv::FONT_HERSHEY_SIMPLEX, 0.3, common::GREEN);

            // Store floor Y position (if it's higher in image -> lower value)
            if (bRect.y < floorY) {
                floorY = bRect.y;
                TRACE_P(pos_, " \tupdated floor = %d", floorY);
            }
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
            } else {
                TRACE_P(pos_, "* EllipseFitting::findLeafContour_(): Comparing "
                              "(%d, %d) [%d x %d] and (%d, %d) [%d x %d]",
                        bRect.x, bRect.y, bRect.width, bRect.height,
                        rects[rectIdx].second.x, rects[rectIdx].second.y,
                        rects[rectIdx].second.width, rects[rectIdx].second.height);

                if (better(bRect, rects[rectIdx].second)) {
                    rects.insert(rects.begin() + rectIdx, std::make_pair(contourIdx, bRect));
                    TRACE_P(pos_, "* EllipseFitting::findLeafContour_(): inserted at index %d", rectIdx);
                }
                else {
                    TRACE_P(pos_, "* EllipseFitting::findLeafContour_(): not inserted at index %d", rectIdx);
                    inserted = false;
                }
            }
        }
        if (!inserted) {
            TRACE_P(pos_, "\tinserted at the end");
            rects.push_back(std::make_pair(contourIdx, bRect));
        }
    }

    TRACE_P(pos_, "* EllipseFitting::findLeafContour_(): last contour check");
    int total = 0, lastInserted = -1;
    for (uint j = 0; j < rects.size(); ++j) {
        if (rects[j].second.empty())
            continue;

        if (rects[j].second.y >= floorY) { // CASE C
            TRACE_P(pos_, "\tx discarded soil contour (%d, %d) [%d x %d]",
                    rects[j].second.x, rects[j].second.y,
                    rects[j].second.width, rects[j].second.height);
            cv::rectangle(contoursOut, rects[j].second, common::MAGENTA);
//            cv::putText(contoursOut, "C",
//                        cv::Point(rects[j].second.x, rects[j].second.y),
//                        cv::FONT_HERSHEY_SIMPLEX, 0.3, common::GREEN);
        } else if ((lastInserted != -1) && (rects[j].second.y > rects[lastInserted].second.y)) {
            // CASE D
            TRACE_P(pos_, "\tx discarded top contour (%d, %d) [%d x %d]",
                    rects[j].second.x, rects[j].second.y,
                    rects[j].second.width, rects[j].second.height);
            cv::rectangle(contoursOut, rects[j].second, common::MAGENTA);
//            cv::putText(contoursOut, "D",
//                        cv::Point(rects[j].second.x, rects[j].second.y),
//                        cv::FONT_HERSHEY_SIMPLEX, 0.3, common::GREEN);
        } else if ((lastInserted != -1) && (rects[lastInserted].second.y - // CASE E
                   (rects[j].second.y + rects[j].second.width) > MAX_CONTOUR_DISTANCE)) {
            TRACE_P(pos_, "\tx discarded top leaf contour (%d, %d) [%d x %d]",
                    rects[j].second.x, rects[j].second.y,
                    rects[j].second.width, rects[j].second.height);
            cv::rectangle(contoursOut, rects[j].second, common::MAGENTA);
//            cv::putText(contoursOut, "E",
//                        cv::Point(rects[j].second.x, rects[j].second.y),
//                        cv::FONT_HERSHEY_SIMPLEX, 0.3, common::GREEN);
        } else {
            TRACE_P(pos_, "\t* valid contour (%d, %d) [%d x %d]",
                    rects[j].second.x, rects[j].second.y,
                    rects[j].second.width, rects[j].second.height);

            leafContour.insert(leafContour.end(), contours[rects[j].first].begin(),
                    contours[rects[j].first].end());

            cv::rectangle(contoursOut, rects[j].second, common::GREEN);
            cv::putText(contoursOut, std::to_string(total),
                        cv::Point(rects[j].second.x, rects[j].second.y),
                        cv::FONT_HERSHEY_SIMPLEX, 0.3, common::MAGENTA);
            total++; lastInserted = j;
        }
    }

    TRACE_P(pos_, "< EllipseFitting::findLeafContour_(%d, %s)", pos_,
          side_ == common::LEFT ? "left" : "right");
}
//------------------------------------------------------------------------------
void EllipseFitting::fitEllipse_(const cv::Mat &input, cv::Mat &ellipseOut,
                                 cv::Mat &contoursOut, float &angle) {
    assert(!roi_.empty());
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Point> leafContour;

    // Initialize references to region of interest for input and outputs
    cv::Mat inputRef, ellipseRef, contoursRef;
    inputRef = input(roi_);
    ellipseRef = ellipseOut(roi_);
    contoursRef = contoursOut(roi_);

    // Finds contours storing both endpoints of each segment of a contour
    findContours(inputRef, contours, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);
    drawContours(contoursRef, contours, -1, common::RED);
    findLeafContour_(contours, leafContour, contoursRef);

    if (leafContour.size() < 5) {
        TRACE_ERR("! EllipseFitting::fitEllipse_(%d, %s): not enough points in contour", pos_,
                  side_ == common::LEFT ? "left" : "right");
        return;
    }
    cv::RotatedRect ellipseRect = cv::fitEllipse(leafContour);
    drawEllipse_(ellipseRect, ellipseRef);

    // Data output
    angle = side_ == common::LEFT ? ellipseRect.angle : 180 - ellipseRect.angle;
    TRACE_P(pos_, "* EllipseFitting::fitEllipse_(%d, %s): angle = %.4f", pos_,
            side_ == common::LEFT ? "left" : "right", static_cast<double>(angle));
}
//------------------------------------------------------------------------------
void EllipseFitting::checkAngle_(float currentAngle, float lastAngle) {
    if (lastAngle < 0) return;
    if (abs(currentAngle - lastAngle) > MAX_DIFFERENCE) {
        TRACE_ERR("* EllipseFitting::checkAngle_(%d, %s): angle difference "
                  "suspiciously big: current angle = %.4f previous angle = %.4f",
                  pos_, side_ == common::LEFT ? "left" : "right",
                  static_cast<double>(currentAngle), static_cast<double>(lastAngle));
        dump_ = true;
    }
}
//------------------------------------------------------------------------------
void EllipseFitting::process(const cv::Mat &leaf,
                             cv::Mat &ellipseDrawing,
                             cv::Mat &contoursDrawing,
                             float &angle, common::Side side) {
    TRACE_P(pos_, "> EllipseFitting::process(%d, %s)", pos_,
            side == common::LEFT ? "left" : "right");

    // Store current side
    side_ = side;

    // Store copy of input
    cv::Mat leafInput = leaf.clone();

    // To BGR
    cv::cvtColor(leaf, ellipseDrawing, cv::COLOR_GRAY2BGR);
    contoursDrawing = ellipseDrawing.clone();

    fitEllipse_(leafInput, ellipseDrawing,
                contoursDrawing,
                angle);

    TRACE_P(pos_, "< EllipseFitting::process(%d, %s)", pos_,
            side == common::LEFT ? "left" : "right");

    if (side == common::LEFT) {

        DUMP_P(pos_, contoursDrawing(roi_), wd_, "contours_l_%d.png", pos_);

        if (pos_ >= 0 && pos_ <= 30) {
            DUMP(contoursDrawing(roi_), wd_, "contours_l_%d.png", pos_);
            DUMP(ellipseDrawing(roi_), wd_, "ellipse_l_%d.png", pos_);
        }

        checkAngle_(angle, lastLAngle_);
        lastLAngle_ = angle;

        DUMP_IF(dump_, contoursDrawing(roi_), wd_, "debug_contours_l_%d.png", pos_);
        DUMP_IF(dump_, ellipseDrawing(roi_), wd_, "debug_ellipse_l_%d.png", pos_);
        dump_ = false;
    } else {
        DUMP_P(pos_, contoursDrawing(roi_), wd_, "contours_r_%d.png", pos_);

        checkAngle_(angle, lastRAngle_);
        lastRAngle_ = angle;

        DUMP_IF(dump_, contoursDrawing(roi_), wd_, "debug_contours_r_%d.png", pos_);
        DUMP_IF(dump_, ellipseDrawing(roi_), wd_, "debug_ellipse_r_%d.png", pos_);
        dump_ = false;

        pos_++;
    }
}
//------------------------------------------------------------------------------
