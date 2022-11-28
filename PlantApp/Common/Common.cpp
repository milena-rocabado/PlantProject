#include "Common.h"

namespace utils {

std::string getDirectoryPath(std::string path) {
    size_t last_slash = path.find_last_of("/");
    if (last_slash != std::string::npos) {
        return path.substr(0, last_slash + 1);
    } else
        return "";
}

}


