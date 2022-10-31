#ifndef BASEANALYZER_H
#define BASEANALYZER_H

#include <opencv2/opencv.hpp>

void analyzer_process(const cv::Mat &input, cv::Mat &output);

void run_base_analyzer();

#endif // BASEANALYZER_H
