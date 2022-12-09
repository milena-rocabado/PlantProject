#ifndef ANALYZERMANAGER_H
#define ANALYZERMANAGER_H

#include <opencv2/opencv.hpp>
#include <fstream>
#include "Enums.h"

#include "PreProcessing.h"
#include "DayOrNight.h"
#include "Thresholding.h"
#include "LeafSegmentation.h"
#include "EllipseFitting.h"

class AnalyzerManager
{
public:
    AnalyzerManager();

    bool setInputPath(std::string inputPath);

    bool initialize();

    void run();

    // Aux Functions -----------------------------------------------------------

    int getMaxFrames();

    void setROI(cv::Rect roi) { roi_ = roi; }

    void setInitialPosition(int initPos) { initPos_ = initPos; }

    void setEndPosition(int endPos) { endPos_ = endPos; }

    void setStartInterval(common::Interval interval) { interval_ = interval; }

    bool setOutputDirectory(std::string path);

    std::string getOutputDirectory();

    std::string getOutputFilename();

    cv::Mat getFrameFromVideo();

private:
    void dumpDataToStream();

private:
    // Constants ---------------------------------------------------------------

    static inline constexpr int TIME_BAR_HEIGHT { 20 };

private:
    // Members -----------------------------------------------------------------

    // Video input
    cv::VideoCapture video_;
    // Region Of Interest
    cv::Rect roi_;
    // Working directory
    std::string workingDir_;
    // Filename for output data
    std::string outDataFn_;
    // Output stream for output data
    std::ofstream outDataFs_;

    // Initial position
    int initPos_;
    // End position
    int endPos_;
    // Current position in frames (zero indexed)
    int pos_;
    // Current interval (day/night)
    common::Interval interval_;

    // Containers --------------------------------------------------------------

    // Frame input from video
    cv::Mat input_;
    // Pre-processing output
    cv::Mat preOut_;
    // Thresholding output
    cv::Mat threshOut_;
    // Leaf segmentation outputs
    cv::Mat leftLeaf_;
    cv::Mat rightLeaf_;
    cv::Mat stem_;
    // Ellipse fitting outputs
    cv::Mat leftEllipse_;
    cv::Mat rightEllipse_;

    // Data output
    common::OutputDataContainer outDataContainer_;

    // Stages ------------------------------------------------------------------
    std::unique_ptr<PreProcessing> preProcessing_;
    std::unique_ptr<DayOrNight> dayOrNight_;
    std::unique_ptr<Thresholding> thresholding_;
    std::unique_ptr<LeafSegmentation> leafSegmentation_;
    std::unique_ptr<EllipseFitting> ellipseFitting_;

};

#endif // ANALYZERMANAGER_H
