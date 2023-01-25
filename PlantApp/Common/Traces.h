#ifndef TRACES_H
#define TRACES_H

#ifdef TRACE_ON

#include <iostream>

#define N_TRACE (100)

#define TRACE(...) { \
    char buf__[256]; \
    std::sprintf(buf__, __VA_ARGS__); \
    std::cout << std::string(buf__) << std::endl; \
}

#define TRACE_ERR(...) { \
    char buf__[256]; \
    std::sprintf(buf__, __VA_ARGS__); \
    std::cout << std::string(buf__) << std::endl; \
}

#define TRACE_IF(b__, ...) { \
    if (b__) { \
        TRACE(__VA_ARGS__); \
    } \
}

#define TRACE_P(pos__, ...) { \
    TRACE_IF(/*pos__ % N_TRACE == 0 ||*/ pos__ == 150 || pos__ == 750, __VA_ARGS__); \
}

#else

#define TRACE(...) {}
#define TRACE_ERR(...) { \
    char buf__[256]; \
    std::sprintf(buf__, __VA_ARGS__); \
    std::cerr << std::string(buf__) << std::endl; \
}
#define TRACE_IF(b__, ...) {}
#define TRACE_P(pos__, ...) {}

#endif

#ifdef DUMP_ON

#define DUMP(mat__, wd__, file__, ...) { \
    char buf__[256]; \
    std::sprintf(buf__, file__, __VA_ARGS__); \
    cv::imwrite(std::string(wd__) + std::string(buf__), mat__); \
}

#define DUMP_IF(b__, mat__, wd__, file__, ...) { \
    if (b__) { \
        DUMP(mat__, wd__, file__, __VA_ARGS__); \
    } \
}

#define DUMP_P(pos__, mat__, wd__, file__, ...) { \
    DUMP_IF((pos__ % N_TRACE == 0||pos__ == 150 || pos__ == 750 || pos__ == 216 || pos__ == 11), mat__, wd__, file__, __VA_ARGS__); \
}

#else

#define DUMP(mat__, wd__, file__, ...) {}
#define DUMP_P(pos__, mat__, wd__, file__, ...) {}
#define DUMP_IF(b__, mat__, wd__, file__, ...) {}

#endif

#endif // TRACES_H
