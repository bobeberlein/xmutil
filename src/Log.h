// Log.h - functions for emitting logging information - wraps writing to stderr.
#ifndef _XMUTIL_LOG_H
#define _XMUTIL_LOG_H
#include <stdio.h>

// for use by bison-generated parser
void XmutilLogf(FILE* f, char* fmt, ...);
void log(const char *msgFmt, ...);

#endif
