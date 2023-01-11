#include "DayOrNight.h"
#include "Traces.h"
#include "Plots.h"

//------------------------------------------------------------------------------
DayOrNight::DayOrNight(): initPos_(-1), lastPos_(-1)
{}
//------------------------------------------------------------------------------
void DayOrNight::dump_brightness_plot() {
    cv::Mat plot;
    utils::plotVector(brightnessVec_, plot);
    DUMP(plot, wd_, "brightness_plot_%d.png", utils::VEC_PLOT_ACCUM);

    brightnessVec_.clear();
}
//------------------------------------------------------------------------------
bool DayOrNight::dayOrNight_(const cv::Mat &input) {
    assert(initPos_ != -1);
    assert(input.channels() == 1);

    bool found { false };

    if (pos_ <= lastPos_) { // After backtracking
        TRACE("* day_or_night_(%d): returning", pos_);
        return found;
    }

    double brightness = mean(input)[0];
    brightnessVec_.push_back(brightness);

    if (pos_ == initPos_) {
        // First call, initialize values
        lastBr_ = brightness;
        lastPos_ = pos_;
        timesSurpassed_ = 0;
        breakpoints_.push_back(pos_);
    } else {
        // Calculate diference in brightness since last pos_
        double dif = abs(brightness - lastBr_);

        if (dif > TOLERANCE) {
            // Increment times tolerance surpassed
            timesSurpassed_++;
            TRACE("* day_or_night_(): !! (%d) lastPos = %d lastBr = %.2f"
                  " pos = %d brightness = %.2f diference = %.2f\n",
                  timesSurpassed_, lastPos_, lastBr_, pos_, brightness, dif);
        } else {
            TRACE_IF(timesSurpassed_ != 0, "* day_or_night(): -- (%d) lastPos = %d lastBr = %.2f"
                     " pos = %d brightness = %.2f diference = %.2f\n",
                     timesSurpassed_, lastPos_, lastBr_, pos_, brightness, dif);

            // Tolerance not surpassed, update lastBr
            lastBr_ = brightness;
            lastPos_ = pos_;
            timesSurpassed_ = 0;
        }

        // Tolerance surpassed MIN_TIMES_SURPASSED times
        if (timesSurpassed_ == MIN_TIMES_SURPASSED) {
            TRACE("* day_or_night(): posible breakpoint at position %d\n", lastPos_+1);

            // If last period is long enough to be full day/night, store breakpoint
            if (pos_ - breakpoints_[breakpoints_.size() - 1] > INTERVAL_MIN_LENGTH) {
                // First position of new interval
                breakpoints_.push_back(++lastPos_);
                found = true;
                TRACE("* day_or_night(): ### breakpoint found: %d\n", lastPos_);
            }

            // Restart to this point
            lastBr_ = brightness;
            lastPos_ = pos_;
            timesSurpassed_ = 0;
        }
    }
    return found;
}
//------------------------------------------------------------------------------
int DayOrNight::process(const cv::Mat &input, common::Interval &interval) {

    if (dayOrNight_(input)) {
        interval = !interval;
        pos_ = breakpoints_.back();
        return breakpoints_.back();
    }

    pos_++;
    return -1;
}
