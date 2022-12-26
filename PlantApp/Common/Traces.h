#ifndef TRACES_H
#define TRACES_H

#ifdef TRACE_ON

#define N_TRACE (150)

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
        char buf__[256]; \
        std::sprintf(buf__, __VA_ARGS__); \
        std::cout << std::string(buf__) << std::endl; \
    } \
}

#define TRACE_P(pos__, ...) { \
    if (pos__ % N_TRACE == 0) { \
        char buf__[256]; \
        std::sprintf(buf__, __VA_ARGS__); \
        std::cout << std::string(buf__) << std::endl; \
    } \
}

#define DUMP(mat__, wd__, file__, ...) { \
    char buf__[256]; \
    std::sprintf(buf__, file__, __VA_ARGS__); \
    cv::imwrite(std::string(wd__) + std::string(buf__), mat__); \
}

#define DUMP_P(pos__, mat__, wd__, file__, ...) { \
    if (pos__ % N_TRACE == 0) { \
        char buf__[256]; \
        std::sprintf(buf__, file__, __VA_ARGS__); \
        cv::imwrite(std::string(wd__) + std::string(buf__), mat__); \
    } \
}

#define DUMP_IF(b__, mat__, wd__, file__, ...) { \
    if (b__) { \
        char buf__[256]; \
        std::sprintf(buf__, file__, __VA_ARGS__); \
        cv::imwrite(std::string(wd__) + std::string(buf__), mat__); \
    } \
}

//#define DUMP_P(pos__, mat__, ...) { \
//    if (pos__ % N_TRACE == 0) { \
//        char buf__[256]; \
//        std::sprintf(buf__, __VA_ARGS__); \
//        imwrite(std::string(DUMP_WD) + std::string(buf__), mat__); \
//    } \
//}

//#define DUMP_IF(b__, mat__, ...) { \
//    if (b__) { \
//        char buf__[256]; \
//        std::sprintf(buf__, __VA_ARGS__); \
//        imwrite(std::string(DUMP_WD) + std::string(buf__), mat__); \
//    } \
//}

#else

#define TRACE(...) ;
#define TRACE_ERR(...) { \
    char buf__[256]; \
    std::sprintf(buf__, __VA_ARGS__); \
    std::cerr << std::string(buf__) << std::endl; \
}
#define TRACE_IF(b__, ...) ;
#define TRACE_P(pos__, ...) ;

#define DUMP(b__, mat__, ...) ;
#define DUMP_IF(b__, mat__, ...) ;

#endif

#endif // TRACES_H
