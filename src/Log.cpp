#include <cstdarg>
#include <cstdio>
#include <cstdlib>

#include "Log.h"

#if defined WIN32 || defined __APPLE__
#define ATTRIBUTE_PRINTF
#else
// this gives us better compiler error messages for callers
#define ATTRIBUTE_PRINTF __attribute__ ((format(printf, 1, 2)))
#endif

void ATTRIBUTE_PRINTF
XmutilLogf(FILE *f, const char* msg_fmt, ...) {
	va_list args;
	va_start(args, msg_fmt);
	vfprintf(f, msg_fmt, args);
	va_end(args);
}
void ATTRIBUTE_PRINTF
log(const char *msg_fmt, ...) {
	va_list args;
	va_start(args, msg_fmt);
	vprintf(msg_fmt, args);
	va_end(args);
}
