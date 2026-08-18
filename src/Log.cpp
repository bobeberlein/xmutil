#include "Log.h"

#include <cstdarg>
#include <cstdio>
#include <cstdlib>

#if defined WIN32 || defined __APPLE__ || defined __EMSCRIPTEN__
#define ATTRIBUTE_PRINTF
#define ATTRIBUTE_PRINTF_F
#else
// this gives us better compiler error messages for callers
#define ATTRIBUTE_PRINTF __attribute__((format(printf, 1, 2)))
// the FILE* variant has the format string as its second argument
#define ATTRIBUTE_PRINTF_F __attribute__((format(printf, 2, 3)))
#endif

void ATTRIBUTE_PRINTF_F XmutilLogf(FILE *f, const char *msg_fmt, ...) {
  va_list args;
  va_start(args, msg_fmt);
  vfprintf(f, msg_fmt, args);
  va_end(args);
}
// log writes to stderr, not stdout: every call site is a diagnostic or warning,
// and the CLI's --stdio mode uses stdout as the converted-document channel
// (Main.cpp streams the result there directly, never through log). Emitting a
// reader/writer diagnostic on stdout would corrupt that document stream and
// leave a downstream consumer with invalid output and no error signal.
void ATTRIBUTE_PRINTF log(const char *msg_fmt, ...) {
  va_list args;
  va_start(args, msg_fmt);
  vfprintf(stderr, msg_fmt, args);
  va_end(args);
}
