#pragma once

#if defined(_MSC_VER) && defined(_DEBUG)
#include <crtdbg.h>
#include <stdlib.h>
#endif

// The MSVC debug CRT reports assert()/STL-checked-iterator failures through a
// modal dialog, which hangs unattended runs (ctest, CI) and hides the message
// from the console. Route those reports to stderr instead. No-op elsewhere.
inline void RouteCrtReportsToConsole() {
#if defined(_MSC_VER) && defined(_DEBUG)
  for (int type : {_CRT_WARN, _CRT_ERROR, _CRT_ASSERT}) {
    _CrtSetReportMode(type, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(type, _CRTDBG_FILE_STDERR);
  }
  _set_error_mode(_OUT_TO_STDERR);
  // Suppress the "abort() has been called" dialog and the WER report.
  _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
#endif
}
