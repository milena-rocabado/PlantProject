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

// noooooooo
#include <chrono>

class AnalyzerManager
{
public:
    AnalyzerManager();

    bool setInputPath(std::string inputPath);

    bool setOutputDirectory(std::string path);

    bool initialize();

    void run();

    // Setter functions --------------------------------------------------------

    void setROI(cv::Rect roi) { roi_ = roi; }

    void setPotPosition(int pos);

    void setInitialPosition(int initPos) { initPos_ = initPos; }

    void setEndPosition(int endPos) { endPos_ = endPos; }

    void setStartInterval(common::Interval interval) { interval_ = interval; }

    void setVideoOutputFlag(bool flag) { videoOutputFlag_ = flag; }

    // Getter functions --------------------------------------------------------

    cv::Rect getROI() { return roi_; }

    int getMaxFrames();

    std::string getOutputDirectory();

    std::string getOutputFilename();

    cv::Mat getFrameFromVideo();

    int getPotPosition();

    void resetROI();

    // Subscriber --------------------------------------------------------------
    class Subscriber {
    public:
        virtual ~Subscriber();

        virtual void updateProgress(double progress) = 0;
    };

    void setSubscriber(Subscriber *sub) { subscriber_ = sub; }

private:
    void dumpDataToStream_();

    void dumpVideoOutput_();

    void updateSubscriber_(int pos);

    void initializeWindows_();

private:
    // Constants ---------------------------------------------------------------

    static inline constexpr int TIME_BAR_HEIGHT { 20 };

    static constexpr int WIN_NUM { 6 };

    static const std::string WIN_NAMES[WIN_NUM];

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

    // For video output
    bool videoOutputFlag_;
    int msecs_;
    // Array of mats that will be shown in video output;
    cv::Mat *outputs_[WIN_NUM];

    // For progress updates
    Subscriber *subscriber_;

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
