#ifndef ENUMS_H
#define ENUMS_H

#include <ostream>

namespace common {

//------------------------------------------------------------------------------
enum Interval {
    DAY,
    NIGHT
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

typedef OutputData_t OutputDataContainer[CONTAINER_SIZE];

}
#endif // ENUMS_H