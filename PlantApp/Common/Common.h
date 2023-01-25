#ifndef ENUMS_H
#define ENUMS_H

#include <ostream>
#include <vector>

namespace common {

//------------------------------------------------------------------------------

enum Interval {
    NIGHT,
    DAY,
    INTERVAL_NUM
};

Interval operator!(Interval interval);

std::ostream& operator<<(std::ostream &os, const Interval &interval);

//------------------------------------------------------------------------------

enum Side {
    LEFT,
    RIGHT
};

std::ostream& operator<<(std::ostream &os, const Side &side);

// Output data container -------------------------------------------------------
typedef struct {
    Interval interval;
    int pos;
    float leftAngle;
    float rightAngle;
} OutputData_t;

static constexpr int CONTAINER_SIZE { 5 };

typedef std::vector<OutputData_t> OutputDataContainer;

}
#endif // ENUMS_H
