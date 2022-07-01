#include <cstdarg>
#include <cstdio>
#include <cstdlib>

#include "Log.h"

#ifdef WIN32
#define ATTRIBUTE_PRINTF
#else
// this gives us better compiler error messages for callers
#define ATTRIBUTE_PRINTF __attribute__ ((format(printf, 1, 2)))
#endif

void ATTRIBUTE_PRINTF
log(const char *msg_fmt, ...) {
    char *msg;
    va_list args;
    int ret;

    va_start(args, msg_fmt);
    ret = vasprintf(&msg, msg_fmt, args);
    va_end(args);

    if (ret == -1) {
        fprintf(stderr, "log: vasprintf failed.\n");
    } else {
        fprintf(stderr, "%s\n", msg);
    }

    free(msg);
}
