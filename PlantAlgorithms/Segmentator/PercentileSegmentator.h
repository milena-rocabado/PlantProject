#ifndef PERCENTILESEGMENTATOR_H
#define PERCENTILESEGMENTATOR_H

#include <opencv2/opencv.hpp>

#define PERCENTILE 0.206638 + 0.02

/**
 * @brief Takes full size frame input, returns ROI plant mask
 * @param input
 * @param output
 */
int percentile_process(const cv::Mat &input, cv::Mat &output);


void determine_percentile();

void determine_percentile_dn();

/**
 * @brief Generates output video with ROI plant mask for each frame from
 * input video.
 */
void run_percentile_segmentator();

#endif // PERCENTILESEGMENTATOR_H
