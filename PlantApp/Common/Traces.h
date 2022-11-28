#ifndef TRACES_H
#define TRACES_H

#ifdef TRACE_ON

#define TRACE(...) { fprintf (stdout, __VA_ARGS__); }
#define TRACE_ERR(...) { fprintf (stderr, __VA_ARGS__); }
#define TRACE_IF(b, ...) { if (b) fprintf (stderr, __VA_ARGS__); }
#define DUMP(b, mat_, ...) { if (b) { \
    char buf_[256]; \
    std::sprintf(buf_, __VA_ARGS__); \
    imwrite(std::string(DUMP_WD) + std::string(buf_), mat_); \
    } \
}

#endif

#endif // TRACES_H
