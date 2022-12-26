#include "LeafSegmentation.h"
#include "Traces.h"

//------------------------------------------------------------------------------
LeafSegmentation::LeafSegmentation() :
    potPosition_(DEFAULT_POT_POSITION)
{}
//------------------------------------------------------------------------------
bool LeafSegmentation::isStem(const cv::Vec3b &px) {
    return px[0] == 0 && px[1] == 0 && px[2] == 0;
}
//------------------------------------------------------------------------------
void LeafSegmentation::colorOverLeaves_(const cv::Point &point, cv::Mat &left, cv::Mat &right) {
    if (point.x != -1) {
        // If stem found, color over opposite side
        // From point to end of row
        line(left, { point.x, point.y }, { left.cols, point.y }, BG_COLOR);
        // From beginning of row to point
        line(right, { 0, point.y }, { point.x, point.y }, BG_COLOR);
    } else {
        // Else, color over whole row
        line(left, { 0, point.y }, { left.cols, point.y }, BG_COLOR);
        line(right, { 0, point.y }, { right.cols, point.y }, BG_COLOR);
    }
}
//------------------------------------------------------------------------------
void LeafSegmentation::hidePlantPot_(cv::Mat &image) {
    cv::Rect pot(0, potPosition_, image.cols, image.rows - potPosition_ - TIMEBAR_HEIGHT);
    cv::rectangle(image, pot, BG_COLOR, cv::FILLED);
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

    // Stop at this row
    int maxRow = potPosition_ - roi_.y;

    TRACE_P(pos_,"* LeafSegmentation::process(%d): first loop start, maxRow = %d", pos_, maxRow);
    // To start, search whole row
    for (row = 0; !found && first && row < maxRow; row++) {
        for (int col = 0; col < ref.cols; col++) {
            cv::Vec3b &pixel = ref.at<cv::Vec3b>(row, col);
            if ((found = isStem(pixel))) {
                // Color stem red
                pixel = RED;
                TRACE_P(pos_,"* LeafSegmentation::search_(%d): pixel found: (%d, %d)", pos_, row, col);
                if (first) {
                    TRACE_P(pos_,"* LeafSegmentation::search_(%d): it's the first!!", pos_);
                    // Store position
                    stem_[row] = col;
                    // Color over leaves
                    colorOverLeaves_({col, row}, lRef, rRef);
                    // Update last position
                    last = col;
                    // Next stem pxs found are not first
                    first = false;
                } else {
                    TRACE_P(pos_,"* LeafSegmentation::search_(%d): not the first", pos_);
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

    for (; row < maxRow; row++) {
        found = false;

        // Search directly under last pos
        cv::Vec3b &pixel = ref.at<cv::Vec3b>(row, last);
        if ((found = isStem(pixel))) {
            // Stem color red
            pixel = RED;
            // Store position
            stem_[row] = last;
            // Color over leaves
            colorOverLeaves_({last, row}, lRef, rRef);
            // Stem found
            radius = 1;

            // Found, but keep searching left and right
            bool l { true }, r { true };
            for (int k = 1; (l || r) && k < MAX_WIDTH; k++) {
                if (l) {
                    cv::Vec3b &px_l = ref.at<cv::Vec3b>(row, last - k);
                    if (isStem(px_l)) {
                        // Color stem red
                        px_l = RED;
                        // Color over stem px in leaf output
                        lRef.at<uchar>(row, last - k) = BG_COLOR;
                        rRef.at<uchar>(row, last - k) = BG_COLOR;
                    }
                    else l = false; // stop searching left
                }
                if (r) {
                    cv::Vec3b &px_r = ref.at<cv::Vec3b>(row, last + k);
                    if (isStem(px_r)) {
                        // Color stem red
                        px_r = RED;
                        // Color over stem px in leaf output
                        lRef.at<uchar>(row, last + k) = BG_COLOR;
                        rRef.at<uchar>(row, last + k) = BG_COLOR;
                    }
                    else r = false; // stop searching right
                }
            }

            // Finished row, next
            continue;
        } else // Next pixel not directly under
            pixel = MAGENTA;

        // Search near last
        for (int j = 1; !found && j <= radius; j++) {

            // To the left
            cv::Vec3b &px_l = ref.at<cv::Vec3b>(row, last - j);
            if ((found = isStem(px_l))) {
                last = last - j;
                // Stem color red
                px_l = RED;
                // Store position
                stem_[row] = last;
                // Color over leaves
                colorOverLeaves_({last, row}, lRef, rRef);
                // Found, but keep searching left
                for (int k = 1; k < MAX_WIDTH; k++) {
                    cv::Vec3b &px = ref.at<cv::Vec3b>(row, last - k);
                    if (isStem(px)){
                        // Color stem red
                        px = RED;
                        // Color over stem px in left leaf output
                        lRef.at<uchar>(row, last - k) = BG_COLOR;
                    } else break; // stop searching left
                }
                // Done, next row
                break;
            } else px_l = CYAN;

            // To the right
            cv::Vec3b &px_r = ref.at<cv::Vec3b>(row, last + j);
            if ((found = isStem(px_r))) {
                last = last + j;
                // Stem color red
                px_r = RED;
                // Store position
                stem_[row] = last;
                // Color over other leaf
                colorOverLeaves_({last, row}, lRef, rRef);
                // Keep searching right
                for (int k = 1; k < MAX_WIDTH; k++) {
                    cv::Vec3b &px = ref.at<cv::Vec3b>(row, last + k);
                    if (isStem(px)){
                        // Color stem red
                        px = RED;
                        // Color over stem px in right leaf output
                        rRef.at<uchar>(row, last + k) = BG_COLOR;
                    } else break; // stop searching right
                }
                // Done, next row
                break;
            } else px_r = GREEN;
        }

        // For rows where stem disappears, not found after searching within radius
        if (! found) {
            TRACE_P(pos_,"* LeafSegmentation::search_(%d): row %d, not found with radius %d", pos_, row, radius);
            radius += 2;
            // Cover whole row -> not needed because close operation cleans
            // leaf and stem residues
            // color_leaves({-1, i}, l_ref, r_ref);
        } else
            // Stem found again
            radius = 1;
    } // end second loop
}
//------------------------------------------------------------------------------
void LeafSegmentation::colorFromVector_(cv::Mat &ref, cv::Mat &lRef, cv::Mat &rRef) {
    assert(lastStem_.size() != 0);


    for (int row = 0; row < lastStem_.size(); row++) {
        int col = lastStem_[row];

        if (col == -1) {
            // Color whole row -> not necessary
            continue;
        }

        ref.at<cv::Vec3b>(row, col) = RED;
        colorOverLeaves_({col, static_cast<int>(row)}, lRef, rRef);

        // Search stem pixels on each side
        bool l {true}, r {true};
        for (int k = 1; (l || r) && k < MAX_WIDTH; k++) {
            if (l) {
                cv::Vec3b &px = ref.at<cv::Vec3b>(row, col - k);
                if (isStem(px)) {
                    px = RED;
                    // Color over stem px in leaf output
                    lRef.at<uchar>(row, col + k) = BG_COLOR;
                    rRef.at<uchar>(row, col + k) = BG_COLOR;
                }
                else l = false;
            }
            if (r) {
                cv::Vec3b &px = ref.at<cv::Vec3b>(row, col + k);
                if (isStem(px)) {
                    px = RED;
                    // Color over stem px in leaf output
                    lRef.at<uchar>(row, col + k) = BG_COLOR;
                    rRef.at<uchar>(row, col + k) = BG_COLOR;
                }
                else r = false;
            }
        }
    }
}
//------------------------------------------------------------------------------
void LeafSegmentation::cleanUp_(cv::Mat &image, int size) {
    // Morphological operation: close
    cv::Mat element = getStructuringElement(cv::MORPH_RECT, cv::Size(size, size));
    morphologyEx(image, image, cv::MORPH_CLOSE, element);
}
//------------------------------------------------------------------------------
void LeafSegmentation::initialize() {
    firstProcessing_ = true;
}
//------------------------------------------------------------------------------
void LeafSegmentation::process(const cv::Mat &input, cv::Mat &left, cv::Mat &right, cv::Mat &stem) {
    assert(!roi_.empty());

    TRACE_P(pos_, "> LeafSegmentation::process(%d)", pos_);

    // Copy input to output images
    cvtColor(input, stem, cv::COLOR_GRAY2BGR);
    // cvtColor(input, right, COLOR_GRAY2BGR);
    // cvtColor(input, left, COLOR_GRAY2BGR);
    right = input.clone();
    left = input.clone();

    hidePlantPot_(left);
    hidePlantPot_(right);

    // Reference to region of interest
    cv::rectangle(stem, roi_, RED);

    cv::Mat ref = stem(roi_);
    cv::Mat lRef = left(roi_);
    cv::Mat rRef = right(roi_);

    TRACE_P(pos_,"* LeafSegmentation::process(%d): mask(%d x %d) ref(%d x %d)",
            pos_,
            roi_.width, roi_.height,
            ref.cols, ref.rows);

    // Stop at this row
    int maxRow = potPosition_ - roi_.y;

    stem_.clear();
    stem_.resize(static_cast<uint>(maxRow), -1);

    search_(ref, lRef, rRef);

    if (firstProcessing_) {
        stemPosition_ = stem_.back();
        firstProcessing_ = false;
        lastStem_ = stem_;
        TRACE("* LeafSegmentation::process(%d): stemPosition = %d", pos_, stemPosition_);
    } else {
        if (stemPosition_ - MAX_WIDTH <= stem_.back()
                && stem_.back() <= stemPosition_ + MAX_WIDTH) {
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
            hidePlantPot_(left);
            hidePlantPot_(right);
            // Reinitialize references
            ref = stem(roi_);
            lRef = left(roi_);
            rRef = right(roi_);
            // Use stem from previous processing
            colorFromVector_(ref, lRef, rRef);
        }
    }

    cleanUp_(lRef, ELEMENT_SIZE);
    cleanUp_(rRef, ELEMENT_SIZE);

    TRACE_P(pos_, "< LeafSegmentation::process(%d)", pos_);
    pos_++;
}
//------------------------------------------------------------------------------
