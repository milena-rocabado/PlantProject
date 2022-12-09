#ifndef COMMON_H
#define COMMON_H

#include <string>
#include <opencv2/opencv.hpp>

namespace utils {

std::string getDirectoryPath(std::string path);

bool parseFilePath(const std::string &path, std::string &dir, std::string &file);

}
#endif // COMMON_H
