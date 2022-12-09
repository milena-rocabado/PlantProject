#include "DayOrNight.h"
#include "Traces.h"

//------------------------------------------------------------------------------
DayOrNight::DayOrNight(): initPos_(-1), update_(false)
{}
//------------------------------------------------------------------------------
void DayOrNight::process(const cv::Mat &input, common::Interval &interval,
                         common::OutputDataContainer &outData) {
    dayOrNight_(input);
    if (update_) {
        interval = !interval;
        update_data_(outData);
    }
    pos_++;
}
//------------------------------------------------------------------------------
void DayOrNight::dayOrNight_(const cv::Mat &input) {
    assert(initPos_ != -1);

    cv::Mat input1C;
    cv::cvtColor(input, input1C, cv::COLOR_BGR2GRAY);
    double brightness = mean(input1C)[0];
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
            TRACE_P(pos_, "* day_or_night_(): !! (%d) last_pos__ = %d last_br_ = %.2f"
                        " pos_ = %d brightness = %.2f diference = %.2f\n",
                  timesSurpassed_, lastPos_, lastBr_, pos_, brightness, dif);
        } else {
            if (timesSurpassed_!=0){
                TRACE_P(pos_, "* day_or_night(): -- (%d) last_pos__ = %d last_br_ = %.2f"
                                     " pos_ = %d brightness = %.2f diference = %.2f\n",
                      timesSurpassed_, lastPos_, lastBr_, pos_, brightness, dif);
            }

            // Tolerance not surpassed, update last_br_
            lastBr_ = brightness;
            lastPos_ = pos_;
            timesSurpassed_ = 0;
        }

        // Tolerance surpassed MIN_TIMES_SURPASSED times
        if (timesSurpassed_ == MIN_TIMES_SURPASSED) {
            TRACE_P(pos_, "* day_or_night(): TIME_STREAK at pos_ %d\n", lastPos_+1);
            // If last period is long enough to be full day/night, store breakpoint
            if (abs(breakpoints_[breakpoints_.size() - 1] - pos_) > INTERVAL_MIN_LENGTH) {
                breakpoints_.push_back(++lastPos_);
                update_ = true;
                TRACE_P(pos_, "* day_or_night(): ### breakpoint found: %d\n", lastPos_);
            }
            // Restart to this point
            lastBr_ = brightness;
            lastPos_ = pos_;
            timesSurpassed_ = 0;
        }
    }
}
//------------------------------------------------------------------------------
void DayOrNight::update_data_(common::OutputDataContainer &outData) {
    int bp = breakpoints_.back();

    for (int i = 0; i < common::CONTAINER_SIZE; ++i) {
        if (outData[i].pos >= bp)
            outData[i].interval = !outData[i].interval;
    }
    update_ = false;
}
