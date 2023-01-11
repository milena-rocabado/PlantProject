#ifndef ANALYZERMANAGER_H
#define ANALYZERMANAGER_H

#include <opencv2/opencv.hpp>
#include <fstream>
#include "Common.h"

#include "PreProcessing.h"
#include "DayOrNight.h"
#include "Thresholding.h"
#include "LeafSegmentation.h"
#include "EllipseFitting.h"

class AnalyzerManager
{
public:
    // Return codes ------------------------------------------------------------

    enum SetInputRetValues {
        INPUT_SET,
        PATH_NOT_VALID,
        VIDEO_NOT_OPEN
    };

    enum SetOutputRetValues {
        OUTPUT_SET,
        OUTPATH_NOENT,
        OUTPATH_NOACCESS,
        OUTPATH_ERROR
    };

    enum InitializeRetValues {
        INITIALIZED,
        OUTFILE_ERROR
    };

public:

    AnalyzerManager();

    ~AnalyzerManager();

    SetInputRetValues setInputPath(std::string inputPath);

    SetOutputRetValues setOutputDirectory(std::string path);

    InitializeRetValues initialize();

    void run();

    // Setter functions --------------------------------------------------------

    void setROI(cv::Rect roi) { roi_ = roi; }

    void resetROI();

    void setPotPosition(int pos);

    void resetPotPosition();

    void setInitialPosition(int initPos);

    void setEndPosition(int endPos);

    void setStartInterval(common::Interval interval) { interval_ = interval; }

    void setVideoOutputFlag(bool flag) { videoOutputFlag_ = flag; }

    // Getter functions --------------------------------------------------------

    cv::Rect getROI() { return roi_; }

    int getMaxFrames();

    std::string getOutputDirectory();

    std::string getOutputFilename();

    cv::Mat getFrameFromVideo();

    int getPotPosition();

private:
    void dumpDataToStream_();

    void dumpVideoOutput_(int);

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
    cv::Mat leftContours_;
    cv::Mat rightEllipse_;
    cv::Mat rightContours_;

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
