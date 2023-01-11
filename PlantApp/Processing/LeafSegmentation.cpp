#include "LeafSegmentation.h"
#include "Traces.h"
#include "Colors.h"
#include <algorithm>

//------------------------------------------------------------------------------
LeafSegmentation::LeafSegmentation() :
    potPosition_(DEFAULT_POT_POSITION)
{}
//------------------------------------------------------------------------------
bool LeafSegmentation::isStem_(const cv::Vec3b &px) {
    return px[0] == 255 && px[1] == 255 && px[2] == 255;
}
//------------------------------------------------------------------------------
void LeafSegmentation::colorOverLeaves_(const cv::Point &point, cv::Mat &left, cv::Mat &right) {
    // Valid point
    if (point.x != -1){
        // From point to end of row
        line(left, { point.x, point.y }, { left.cols, point.y }, BG_COLOR);
        // From beginning of row to point
        line(right, { 0, point.y }, { point.x, point.y }, BG_COLOR);
    }
    // Invalid point, color whole row
    else {
        line(left, { 0, point.y }, { left.cols, point.y }, BG_COLOR);
        line(right, { 0, point.y }, { right.cols, point.y }, BG_COLOR);
    }
}
//------------------------------------------------------------------------------
void LeafSegmentation::hidePlantPot_(cv::Mat &image) {
    cv::Rect pot(0, potPosition_, image.cols, image.rows - potPosition_ - TIMEBAR_HEIGHT);
    TRACE_P(pos_, "* LeafSegmentation::hidePlantPot_(%d): rect (%d, %d) [%d x %d]",
            pos_, pot.x, pot.y, pot.width, pot.height);
    cv::rectangle(image, pot, BG_COLOR, cv::FILLED);
}
//------------------------------------------------------------------------------
void LeafSegmentation::searchSides_(cv::Mat &ref, cv::Mat &lRef, cv::Mat &rRef,
                                    int row, int last, bool left, bool right) {

    int k;
    for (k = 1; (left || right) && k < MAX_WIDTH; k++) {
        if (left) {
            cv::Vec3b &px_l = ref.at<cv::Vec3b>(row, last - k);
            if (isStem_(px_l)) {
                // Color stem red
                px_l = common::RED;
                // Color over stem px in leaf output
                lRef.at<uchar>(row, last - k) = BG_COLOR;
                rRef.at<uchar>(row, last - k) = BG_COLOR;
            }
            else left = false; // stop searching left
        }
        if (right) {
            cv::Vec3b &px_r = ref.at<cv::Vec3b>(row, last + k);
            if (isStem_(px_r)) {
                // Color stem red
                px_r = common::RED;
                // Color over stem px in leaf output
                lRef.at<uchar>(row, last + k) = BG_COLOR;
                rRef.at<uchar>(row, last + k) = BG_COLOR;
            }
            else right = false; // stop searching right
        }
    }

    // From now on, limit search radius
    if (k == MAX_WIDTH) {
        if (!maxWidthReached_)
            TRACE_P(pos_, "* LeafSegmentation::searchSides_(%d): max width reached,"
                "start limiting radius", pos_);
        maxWidthReached_ = true;
    }
}
//------------------------------------------------------------------------------
void LeafSegmentation::search_(cv::Mat &ref, cv::Mat &lRef, cv::Mat &rRef) {

    // Whether any stem px has been found
    bool found { false };
    // Whether next stem px to be found is first
    bool first { true };
    // Position of first stem px found in last row
    int last { 0 };
    // Current row index
    int row { 0 };
    // Search radius
    int radius { 1 };
    // Maximum width has not been reached yet
    maxWidthReached_ = false;

    auto stemFound = [&stemRef = stem_, &last] (int row, int col, cv::Vec3b &px) {
        px = common::RED;
        last = col;
        stemRef[row] = col;
    };

    TRACE_P(pos_,"* LeafSegmentation::process(%d): first loop start, maxRow = %d",
            pos_, maxRow_);
    // To start, search whole row
    for (row = 0; !found && first && row < maxRow_; row++) {
        for (int col = 0; col < ref.cols; col++) {
            cv::Vec3b &pixel = ref.at<cv::Vec3b>(row, col);
            if ((found = isStem_(pixel))) {
                if (first) {
                    TRACE_P(pos_,"* LeafSegmentation::search_(%d): pixel found: (%d, %d) it's the first!!", pos_, row, col);
                    // Handle pixel and vector + update last
                    stemFound(row, col, pixel);
                    // Color over leaves
                    colorOverLeaves_({col, row}, lRef, rRef);
                    // Next stem pxs found are not first
                    first = false;
                } else {
                    TRACE_P(pos_,"* LeafSegmentation::search_(%d): pixel found: (%d, %d) not the first", pos_, row, col);
                    // Color stem red
                    pixel = common::RED;
                    // Color over stem px in r_ref
                    rRef.at<uchar>(row, col) = BG_COLOR;
                    // for l_ref, colorOverLeaves_ covers all stem pxs
                }
            } else {
                // Found end of stem (first stem px has been found before)
                if (!first) {
                    TRACE_P(pos_,"* LeafSegmentation::search_(%d): first has been found before, therefore end of stem", pos_);
                    break;
                }
            }
        }
        if (first) { // First not found
            TRACE_P(pos_,"* LeafSegmentation::search_(%d): end of row, first pixel not found: (%d)", pos_, row);
            // Cover whole row
            colorOverLeaves_({-1, row}, lRef, rRef);
        }
    } // end first loop

    TRACE_P(pos_,"* LeafSegmentation::search_(%d): first loop end, row = %d", pos_, row);

    for (; row < maxRow_; row++) {
        found = false;

        // Search directly under last pos
        cv::Vec3b &pixel = ref.at<cv::Vec3b>(row, last);
        if ((found = isStem_(pixel))) {
            // Handle pixel and vector
            stemFound(row, last, pixel);
            // Color over leaves
            colorOverLeaves_({last, row}, lRef, rRef);
            // Found, but keep searching left and right
            searchSides_(ref, lRef, rRef, row, last, true, true);
        } else // Next pixel not directly under
            pixel = common::MAGENTA;

        // Search near last
        for (int j = 1; !found && j <= radius; j++) {

            // To the left
            cv::Vec3b &px_l = ref.at<cv::Vec3b>(row, last - j);
            if ((found = isStem_(px_l))) {
                // Handle pixel and vector + update last
                stemFound(row, last - j, px_l);
                // Color over leaves
                colorOverLeaves_({last, row}, lRef, rRef);
                // Found, but keep searching left
                searchSides_(ref, lRef, rRef, row, last, true, false);
                // Done, next row
                break;
            } else px_l = common::CYAN;

            // To the right
            cv::Vec3b &px_r = ref.at<cv::Vec3b>(row, last + j);
            if ((found = isStem_(px_r))) {
                // Handle pixel and vector + update last
                stemFound(row, last + j, px_r);
                // Color over other leaf
                colorOverLeaves_({last, row}, lRef, rRef);
                // Keep searching right
                searchSides_(ref, lRef, rRef, row, last, false, true);
                // Done, next row
                break;
            } else px_r = common::GREEN;
        }

        // For rows where stem disappears, not found after searching within radius
        if (! found) {
            // Increment search radius
            radius = maxWidthReached_ ? std::min(MAX_RADIUS, radius+2) : radius+2;
            // Use last value
            colorOverLeaves_({last, row}, lRef, rRef);
            // To avoid invalid value in vector
            stem_[row] = last;
        } else {
            if (radius != 1)
                TRACE_P(pos_, "* LeafSegmentation::search_(%d): stem found again with radius %d, reset radius", pos_, radius);
            // Stem found again
            radius = 1;
        }
    } // end second loop
}
//------------------------------------------------------------------------------
void LeafSegmentation::colorFromVector_(cv::Mat &ref, cv::Mat &lRef, cv::Mat &rRef) {
    assert(lastStem_.size() != 0);

    int lastVal = -1;
    for (uint row = 0; row < lastStem_.size(); row++) {
        int col = lastStem_[row];

        if (col == -1) {
            col = lastVal;
        } else lastVal = col;

        ref.at<cv::Vec3b>(row, col) = common::RED;
        colorOverLeaves_({col, static_cast<int>(row)}, lRef, rRef);

        // Search stem pixels on each side
        searchSides_(ref, lRef, rRef, row, col, true, true);
    }
}
//------------------------------------------------------------------------------
void LeafSegmentation::cleanUp_(cv::Mat &image) {
    // Morphological operation: open
    cv::Mat element = getStructuringElement(cv::MORPH_RECT,
                                            cv::Size(ELEMENT_SIZE, ELEMENT_SIZE));
    morphologyEx(image, image, cv::MORPH_OPEN, element);

//    // Dilate, to join pieces of leaf
//    element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(7,7));
//    cv::dilate(image, image, element);
}
//------------------------------------------------------------------------------
void LeafSegmentation::initialize() {
    firstProcessing_ = true;
    maxRow_ = potPosition_ - roi_.y;
}
//------------------------------------------------------------------------------
void LeafSegmentation::process(const cv::Mat &input, cv::Mat &left, cv::Mat &right, cv::Mat &stem) {
    assert(!roi_.empty());

    TRACE_P(pos_, "> LeafSegmentation::process(%d)", pos_);

    // Copy input to output images
    cvtColor(input, stem, cv::COLOR_GRAY2BGR);
    right = input.clone();
    left = input.clone();

    // Reference to region of interest
    cv::Mat ref = stem(roi_);
    cv::Mat lRef = left(roi_);
    cv::Mat rRef = right(roi_);

    stem_.clear();
    stem_.resize(static_cast<uint>(maxRow_), -1);

    search_(ref, lRef, rRef);

    if (firstProcessing_) {
        stemPosition_ = stem_.back();
        firstProcessing_ = false;
        lastStem_ = stem_;
        TRACE("* LeafSegmentation::process(%d): stemPosition = %d", pos_, stemPosition_);
    } else {
        if (stemPosition_ - MAX_DIFF <= stem_.back()
                && stem_.back() <= stemPosition_ + MAX_DIFF) {
            TRACE_P(pos_,"* LeafSegmentation::process(%d): stem found is correct", pos_);
            lastStem_ = stem_;
        } else {
            TRACE_P(pos_,"* LeafSegmentation::process(%d): stem found is not correct!!", pos_);
            // Release incorrect mat
            stem.release();
            right.release();
            left.release();
            // Reinitialize mats
            cvtColor(input, stem, cv::COLOR_GRAY2BGR);
            right = input.clone();
            left = input.clone();
            // Reinitialize references
            ref = stem(roi_);
            lRef = left(roi_);
            rRef = right(roi_);
            // Use stem from previous processing
            colorFromVector_(ref, lRef, rRef);
        }
    }

    hidePlantPot_(left);
    cleanUp_(left);
    hidePlantPot_(right);
    cleanUp_(right);

    TRACE_P(pos_, "< LeafSegmentation::process(%d)", pos_);
    pos_++;
}
//------------------------------------------------------------------------------
