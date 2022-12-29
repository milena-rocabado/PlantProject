#include "AnalyzerManager.h"

#include <unistd.h>

#include "Traces.h"
#include "Utils.h"

//------------------------------------------------------------------------------
AnalyzerManager::Subscriber::~Subscriber() {}
//------------------------------------------------------------------------------
const std::string AnalyzerManager::WIN_NAMES[] {
    "right_leaf",
    "thresholding",
    "left_leaf",
    "right_ellipses",
    "stem",
    "left_ellipses",
};
//------------------------------------------------------------------------------
AnalyzerManager::AnalyzerManager()
    : preProcessing_(std::make_unique<PreProcessing>())
    , dayOrNight_(std::make_unique<DayOrNight>())
    , thresholding_(std::make_unique<Thresholding>())
    , leafSegmentation_(std::make_unique<LeafSegmentation>())
    , ellipseFitting_(std::make_unique<EllipseFitting>())
{ }
//------------------------------------------------------------------------------
AnalyzerManager::~AnalyzerManager() {
    if (video_.isOpened())
        video_.release();
}
//------------------------------------------------------------------------------
void AnalyzerManager::setPotPosition(int pos) {
    leafSegmentation_->setPotPosition(pos);
}
//------------------------------------------------------------------------------
void AnalyzerManager::setInitialPosition(int initPos) {
    initPos_ = initPos - 1; // we use 0-index
}
//------------------------------------------------------------------------------
void AnalyzerManager::setEndPosition(int endPos) {
    endPos_ = endPos - 1; // we use 0-index
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int AnalyzerManager::getMaxFrames() {
    assert(video_.isOpened());

    return static_cast<int>(video_.get(cv::CAP_PROP_FRAME_COUNT));
}
//------------------------------------------------------------------------------
std::string AnalyzerManager::getOutputDirectory() {
    assert(video_.isOpened());

    return workingDir_;
}
//------------------------------------------------------------------------------
std::string AnalyzerManager::getOutputFilename() {
    assert(video_.isOpened());

    return workingDir_ + outDataFn_;
}
//------------------------------------------------------------------------------
cv::Mat AnalyzerManager::getFrameFromVideo() {
    assert(video_.isOpened());

    cv::Mat frame;
    video_ >> frame;
    video_.set(cv::CAP_PROP_POS_FRAMES, 0);

    return frame;
}
//------------------------------------------------------------------------------
int AnalyzerManager::getPotPosition() {
    return leafSegmentation_->getPotPosition();
}
//------------------------------------------------------------------------------
void AnalyzerManager::resetROI() {
    assert(video_.isOpened());

    cv::Size inputSize(static_cast<int>(video_.get(cv::CAP_PROP_FRAME_WIDTH)),
                       static_cast<int>(video_.get(cv::CAP_PROP_FRAME_HEIGHT)));
    // Initialize ROI
    cv::Point roiCorner;
    roiCorner.x = inputSize.width / 3;
    roiCorner.y = inputSize.height / 3;

    cv::Size roiSize;
    roiSize.width  = inputSize.width / 3;
    roiSize.height = static_cast<int>(2.0 * inputSize.height / 3.0)
            - AnalyzerManager::TIME_BAR_HEIGHT;

    roi_ = cv::Rect(roiCorner, roiSize);
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool AnalyzerManager::setOutputDirectory(std::string path) {
    assert(video_.isOpened());

    int retVal = access(path.c_str(), R_OK | W_OK);

    if (retVal != 0) {
        if (errno == ENOENT) {
            TRACE_ERR("* AnalyzerManager::setOutputDirectory: Directory \"%s\" "
                      "does not exist", path.c_str());
        }
        else if (errno == EACCES) {
            TRACE_ERR("* AnalyzerManager::setOutputDirectory: Access denied to "
                      "directory \"%s\"", path.c_str())
        }
        return false;
    }

    workingDir_ = path;
    return true;
}
//------------------------------------------------------------------------------
bool AnalyzerManager::setInputPath(std::string inputPath) {
    TRACE("> AnalyzerManager::setInputPath(%s)", inputPath.c_str());

    bool valid = utils::parseFilePath(inputPath, workingDir_, outDataFn_);

    // If invalid path, return error
    if (!valid || workingDir_.empty() || outDataFn_.empty()) {
        TRACE_ERR("* AnalyzerManager::setInputPath: input \"%s\" path not valid",
                  inputPath.c_str());
        return false;
    }

    if (video_.isOpened())
        video_.release();

    video_.open(inputPath);
    // If video capture not open, return error
    if (!video_.isOpened()) {
        TRACE_ERR("* AnalyzerManager::setInputPath: Error opening provided video file \"%s\"",
                  inputPath.c_str());
        return false;
    }

    outDataFn_ += ".csv";

    // Set default ROI
    resetROI();

    TRACE("* AnalyzerManager::setInputPath: workingDir = \"%s\" output data file = \"%s\"",
          workingDir_.c_str(), outDataFn_.c_str());
    TRACE("< AnalyzerManager::setInputPath");
    return true;
}
//------------------------------------------------------------------------------
void AnalyzerManager::initializeWindows_() {
    int rows {2}, cols {3};
    int padding = 10;
    int HSpacing = roi_.width;
    int VSpacing = roi_.height + 10;

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            cv::namedWindow(WIN_NAMES[i*cols + j]);
            cv::moveWindow(WIN_NAMES[i*cols + j],
                    j * HSpacing + (j+1) * padding,
                    i * VSpacing + (i+1) * padding);
            cv::resizeWindow(WIN_NAMES[i*cols + j], roi_.width, roi_.height);
        }
    }

    outputs_[0] = &leftLeaf_;
    outputs_[1] = &threshOut_;
    outputs_[2] = &rightLeaf_;
    outputs_[3] = &leftEllipse_;
    outputs_[4] = &stem_;
    outputs_[5] = &rightEllipse_;
}
//------------------------------------------------------------------------------
bool AnalyzerManager::initialize() {
    assert(video_.isOpened());

    cv::Size inputSize(static_cast<int>(video_.get(cv::CAP_PROP_FRAME_WIDTH)),
                       static_cast<int>(video_.get(cv::CAP_PROP_FRAME_HEIGHT)));

    // Initialize stage classes
    preProcessing_->setInitialPosition(initPos_);
    preProcessing_->setInputSize(inputSize);
    preProcessing_->setROI(roi_);
    preProcessing_->setDumpDirectory(workingDir_ + "Ejemplos/");
    preProcessing_->initialize();

    dayOrNight_->setInitialPosition(initPos_);

    thresholding_->setInitialPosition(initPos_);
    thresholding_->setROI(roi_);
    thresholding_->setInputSize(inputSize);

    leafSegmentation_->setInitialPosition(initPos_);
    leafSegmentation_->setROI(roi_);
    leafSegmentation_->setDumpDirectory(workingDir_ + "Ejemplos/");
    leafSegmentation_->initialize();

    ellipseFitting_->setInitialPosition(initPos_);
    ellipseFitting_->setROI(roi_);

    // Initialize output data stream
    std::string filepath = workingDir_ + outDataFn_;
    outDataFs_.open(filepath);
    if (!outDataFs_ || !outDataFs_.is_open()) {
        TRACE_ERR("* AnalyzerManager::initialize(): unable to open output data file \"%s\"",
                  filepath.c_str());
        return false;
    } else {
        outDataFs_ << "Frame,Day/Night,Left_leaf_angle,Right_leaf_angle" << std::endl;
    }

    // Prepare video output
    if (videoOutputFlag_) {
        msecs_ = static_cast<int>(1000.0 / video_.get(cv::CAP_PROP_FPS));
        initializeWindows_();
    }

    return true;
}
//------------------------------------------------------------------------------
void AnalyzerManager::dumpDataToStream_() {
    int index = initPos_ % common::CONTAINER_SIZE;
    for (int i = 0; i < common::CONTAINER_SIZE; ++i) {
        outDataFs_ << outDataContainer_[index].pos << ","
                   << outDataContainer_[index].interval << ","
                   << outDataContainer_[index].leftAngle << ","
                   << outDataContainer_[index].rightAngle << std::endl;
        index = ++index % common::CONTAINER_SIZE;
    }
}
//------------------------------------------------------------------------------
void AnalyzerManager::dumpVideoOutput_() {
    for (int i = 0; i < WIN_NUM; i++) {
        // ----
        cv::putText((*outputs_[i])(roi_), std::to_string(pos_), cv::Point(0, 25),
                    cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0));
        // ----
        cv::imshow(WIN_NAMES[i], (*outputs_[i])(roi_));
    }
    //_________
    cv::imshow("left-contours", ellipseFitting_->getContoursLOut());
    cv::imshow("right-contours", ellipseFitting_->getContoursROut());
    //_________

    cv::waitKey(msecs_ - 15);
}
//------------------------------------------------------------------------------
void AnalyzerManager::updateSubscriber_(int pos) {
    double progress = pos / endPos_;
    subscriber_->updateProgress(progress);
}
//------------------------------------------------------------------------------
void AnalyzerManager::run() {
    assert(video_.isOpened());

    TRACE("* AnalyzerManager::run(): loop begin");

    video_.set(cv::CAP_PROP_POS_FRAMES, initPos_);

    for (pos_ = initPos_; pos_ <= endPos_; pos_++) {

        // Extract frame
        video_ >> input_;

        if (input_.empty()) {
            TRACE_ERR("* AnalyzerManager::run(): unexpected end at position %d", pos_);
            return;
        }

        int index = pos_ % common::CONTAINER_SIZE;
        outDataContainer_[index].pos = pos_;
        outDataContainer_[index].interval = interval_;

        // Day or night
        dayOrNight_->process(input_, interval_, outDataContainer_);

        // Preprocessing
        preProcessing_->process(input_, preOut_);

        // Thresholding
        thresholding_->process(preOut_, interval_, threshOut_);

        // Segmentation
        leafSegmentation_->process(threshOut_, leftLeaf_, rightLeaf_, stem_);

        // Ellipse fitting
        ellipseFitting_->process(leftLeaf_, threshOut_, leftEllipse_,
                        outDataContainer_[index].leftAngle,
                        common::LEFT);
        ellipseFitting_->process(rightLeaf_, threshOut_, rightEllipse_,
                        outDataContainer_[index].rightAngle,
                        common::RIGHT);

        // Dump data to file stream
        if (index == common::CONTAINER_SIZE - 1)
            dumpDataToStream_();
        // Update subscriber
        if (pos_ % (5 * static_cast<int>(video_.get(cv::CAP_PROP_FPS))) == 0) {
            TRACE("* AnalyzerManager::run(): %d seconds analyzed",
                  pos_ / (5 * static_cast<int>(video_.get(cv::CAP_PROP_FPS))));
            if (subscriber_)
                updateSubscriber_(pos_);
        }

        // Video output
        if (videoOutputFlag_) {
            dumpVideoOutput_();
        }

        // DUMPS
//        DUMP_P(pos_, input_, workingDir_, "Ejemplos/input_%d.png", pos_);
//        DUMP_P(pos_, preOut_, workingDir_, "Ejemplos/preproc_%d.png", pos_);
//        DUMP_P(pos_, threshOut_, workingDir_, "Ejemplos/thresh_%d.png", pos_);
        DUMP_P(pos_, leftLeaf_, workingDir_, "Ejemplos/leaf_l_%d.png", pos_);
        DUMP_P(pos_, rightLeaf_, workingDir_, "Ejemplos/leaf_r_%d.png", pos_);
        DUMP_P(pos_, leftEllipse_, workingDir_, "Ejemplos/ellipse_l_%d.png", pos_);
        DUMP_P(pos_, rightEllipse_, workingDir_, "Ejemplos/ellipse_r_%d.png", pos_);
    }

    if (videoOutputFlag_) {
        cv::destroyAllWindows();
    }
    outDataFs_.close();
}
