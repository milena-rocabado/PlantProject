#ifndef COMMON_H
#define COMMON_H

#include <string>

namespace utils {

std::string getDirectoryPath(std::string path);

bool parseFilePath(const std::string &path, std::string &dir, std::string &file);

}
#endif // COMMON_H
