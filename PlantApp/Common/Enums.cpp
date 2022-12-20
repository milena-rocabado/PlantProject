#include "Enums.h"

namespace common {

Interval operator!(Interval interval) {
    return interval == DAY ? NIGHT : DAY;
}

std::ostream& operator<<(std::ostream& os, const Interval& interval) {
    return os << (interval == DAY ? "Day" : "Night");
}


std::ostream& operator<<(std::ostream &os, const Side &side) {
    return os << (side == LEFT ? "left" : "right");
}

}
