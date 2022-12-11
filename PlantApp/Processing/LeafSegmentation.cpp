#include "LeafSegmentation.h"
#include "Traces.h"

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
    cv::Rect pot(0, POT_POSITION, image.cols, image.rows - POT_POSITION - TIMEBAR_HEIGHT);
    cv::rectangle(image, pot, BG_COLOR, cv::FILLED);
}
//------------------------------------------------------------------------------
void LeafSegmentation::process(const cv::Mat &input, cv::Mat &right, cv::Mat &left, cv::Mat &stem) {
    assert(!roi_.empty());

    TRACE_P(pos_, "> LeafSegmentation::process(%d)\n", pos_);

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

    auto is_stem = [](cv::Vec3b px) -> bool {
        return px[0] == 0 && px[1] == 0 && px[2] == 0;
    };

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

    TRACE_P(pos_,"* LeafSegmentation::process(%d): mask(%d x %d) ref(%d x %d)\n", pos_,
            roi_.width, roi_.height,
            ref.cols, ref.rows);

    // Stop at this row
    int maxRow = POT_POSITION - roi_.y;

    TRACE_P(pos_,"* LeafSegmentation::process(%d): first loop start, maxRow = %d\n", pos_, maxRow);
    // To start, search whole row
    for (row = 0; !found && first && row < maxRow; row++) {
        for (int col = 0; col < ref.cols; col++) {
            cv::Vec3b &pixel = ref.at<cv::Vec3b>(row, col);
            if ((found = is_stem(pixel))) {
                // Color stem red
                pixel = RED;
                TRACE_P(pos_,"* LeafSegmentation::process(%d): pixel found: (%d, %d)\n", pos_, row, col);
                if (first) {
                    TRACE_P(pos_,"* LeafSegmentation::process(%d): it's the first!!\n", pos_);
                    // Color over leaves
                    colorOverLeaves_({col, row}, lRef, rRef);
                    // Update last position
                    last = col;
                    // Next stem pxs found are not first
                    first = false;
                } else {
                    TRACE_P(pos_,"* LeafSegmentation::process(%d): not the first\n", pos_);
                    // Color over stem px in r_ref
                    rRef.at<uchar>(row, col) = BG_COLOR;
                    // for l_ref, colorOverLeaves_ covers all stem pxs
                }
            } else {
                // Found end of stem (first stem px has been found before)
                if (!first) {
                    TRACE_P(pos_,"* LeafSegmentation::process(%d): first has been found before, therefore end of stem\n", pos_);
                    break;
                }
            }
        }
        if (first) { // First not found
            TRACE_P(pos_,"* LeafSegmentation::process(%d): end of row, first pixel not found: (%d)\n", pos_, row);
            // Cover whole row
            colorOverLeaves_({-1, row}, lRef, rRef);
        }
    } // end first loop

    TRACE_P(pos_,"* LeafSegmentation::process(%d): first loop end, row = %d\n", pos_, row);

    for (; row < maxRow; row++) {
        found = false;

        // Search directly under last pos
        cv::Vec3b &pixel = ref.at<cv::Vec3b>(row, last);
        if ((found = is_stem(pixel))) {
            // Stem color red
            pixel = RED;
            // Color over leaves
            colorOverLeaves_({last, row}, lRef, rRef);

            // Found, but keep searching left and right
            bool l { true }, r { true };
            for (int k = 1; (l || r) && k < MAX_WIDTH; k++) {
                if (l) {
                    cv::Vec3b &px_l = ref.at<cv::Vec3b>(row, last - k);
                    if (is_stem(px_l)) {
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
                    if (is_stem(px_r)) {
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
            if ((found = is_stem(px_l))) {
                last = last - j;
                // Stem color red
                px_l = RED;
                // Color over leaves
                colorOverLeaves_({last, row}, lRef, rRef);
                // Found, but keep searching left
                for (int k = 1; k < MAX_WIDTH; k++) {
                    cv::Vec3b &px = ref.at<cv::Vec3b>(row, last - k);
                    if (is_stem(px)){
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
            if ((found = is_stem(px_r))) {
                last = last + j;
                // Stem color red
                px_r = RED;
                // Color over other leaf
                colorOverLeaves_({last, row}, lRef, rRef);
                // Keep searching right
                for (int k = 1; k < MAX_WIDTH; k++) {
                    cv::Vec3b &px = ref.at<cv::Vec3b>(row, last + k);
                    if (is_stem(px)){
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
            TRACE_P(pos_,"* LeafSegmentation::process(%d): row %d, not found with radius %d\n", pos_, row, radius);
            radius += 2;
            // Cover whole row -> not needed because close operation cleans
            // leaf and stem residues
            // color_leaves({-1, i}, l_ref, r_ref);
        } else
            // Stem found again
            radius = 1;
    } // end second loop

    TRACE_P(pos_,"< LeafSegmentation::process(%d)\n", pos_);
    pos_++;
}
//------------------------------------------------------------------------------
