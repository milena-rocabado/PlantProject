#include "AnalyzerManager.h"

#include "Traces.h"

AnalyzerManager::AnalyzerManager() {

}
//------------------------------------------------------------------------------
bool AnalyzerManager::setInputPath(std::string inputPath) {
    TRACE(">AnalyzerManager::setInputPath")
    inputPath_ = inputPath;
    workingDir_ = utils::getDirectoryPath(inputPath);

    // If invalid path, return error
    if (workingDir_.empty()) {
        std::cerr << "setInputPath: input path not valid" << std::endl;
        return false;
    }

    if (video_.isOpened())
        video_.release();

    // If video capture not open, return error
    TRACE("*AnalyzerManager::setInputPath: opening video capture")
    video_.open(inputPath_);
    if (!video_.isOpened()) {
        std::cerr << "setInputPath: Error opening provided video file" << std::endl;
        return false;
    }

    TRACE("<AnalyzerManager::setInputPath")
    return true;
}
//------------------------------------------------------------------------------
void AnalyzerManager::setROI() {

}
//------------------------------------------------------------------------------
void AnalyzerManager::initialize() {
    assert(!inputPath_.empty());
}
