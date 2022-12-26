#include "Utils.h"

namespace utils {

std::string getDirectoryPath(std::string path) {
    size_t lastSlash = path.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        return path.substr(0, lastSlash + 1);
    } else
        return "";
}

bool parseFilePath(const std::string &path, std::string &dir, std::string &file) {
    size_t lastSlash = path.find_last_of("/\\");

    if (lastSlash != std::string::npos) {
        dir.assign(path.substr(0, lastSlash + 1));
    } else {
        dir.clear();
        file.clear();
        return false;
    }

    size_t lastDot = path.find_last_of('.');
    if (lastDot != std::string::npos) {
        int len = lastDot - lastSlash - 1;
        file.assign(path.substr(lastSlash + 1, len));
    } else  {
        dir.clear();
        file.clear();
        return false;
    }

    return true;
}

}


