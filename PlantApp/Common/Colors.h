#ifndef COLORS_H
#define COLORS_H

#include <opencv2/core/mat.hpp>

namespace common {

inline static const cv::Vec3b BLUE  { 255,   0,   0 };
inline static const cv::Vec3b GREEN {   0, 255,   0 };
inline static const cv::Vec3b RED   {   0,   0, 255 };

inline static const cv::Vec3b YELLOW  {   0, 255, 255 };
inline static const cv::Vec3b MAGENTA { 255,   0, 255 };
inline static const cv::Vec3b CYAN    { 255, 255,   0 };

}

#endif // COLORS_H
