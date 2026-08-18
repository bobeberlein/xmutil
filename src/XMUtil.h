#ifndef _XMUTIL_XMUTIL_H
#define _XMUTIL_XMUTIL_H

#include <cstdint>
#include <string>

#include "Log.h"

#ifdef WIN32
// XMUtil.h - globally included - generally for help with
// memory leak detection
//
#if defined(rollyourown) && defined(_DEBUG) && defined(__cplusplus)
#include <new>
extern void AddTrack(void *p, size_t size, const char *file, int line);
extern void RemoveTrack(void *p);
/* specialized placement new to track allocations */
inline void *__cdecl operator new(size_t size, const char *file, int line) {
  void *ptr = (void *)malloc(size);
  if (!ptr)
    throw std::bad_alloc();
  AddTrack(ptr, size, file, line);
  return (ptr);
};
inline void *__cdecl operator new[](size_t size, const char *file, int line) {
  void *ptr = (void *)malloc(size);
  if (!ptr)
    throw std::bad_alloc();
  AddTrack(ptr, size, file, line);
  return (ptr);
};
/* matching placement delete for exception handling */
inline void __cdecl operator delete(void *p, const char *file, int line) {
  RemoveTrack(p);
  free(p);
}
inline void __cdecl operator delete[](void *p, const char *file, int line) {
  RemoveTrack(p);
  free(p);
}
inline void __cdecl operator delete(void *p) {
  RemoveTrack(p);
  free(p);
};
inline void __cdecl operator delete[](void *p) {
  RemoveTrack(p);
  free(p);
};

#define XDEBUG_NEW new (__FILE__, __LINE__)
#define new XDEBUG_NEW
#define XDEBUG_DELETE delete
#define delete XDEBUG_DELETE
#elif defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <stdlib.h>
#define DBG_NEW new (_CLIENT_BLOCK, __FILE__, __LINE__)
#define new DBG_NEW
#endif
#endif

#ifdef WIN32
#define XMUTIL_EXPORT
#else
#define XMUTIL_EXPORT __attribute__((visibility("default")))
#endif

extern "C" {
// returns NULL on error or a string containing XMILE that the caller now owns
XMUTIL_EXPORT char *convert_mdl_to_xmile(const char *mdlSource, uint32_t mdlSourceLen, const char *fileName,
                                         bool isCompact, int isLongName, bool isAsSectors);

// returns NULL on error or a string containing Vensim .mdl that the caller now owns
XMUTIL_EXPORT char *convert_to_mdl(const char *mdlSource, uint32_t mdlSourceLen, const char *fileName, int isLongName);

// returns NULL on error or a string containing XMILE that the caller now owns.
// isLongName and isAsSectors are both accepted for API symmetry with
// convert_mdl_to_xmile and neither is consumed. isLongName: the v1 reader has
// no use for it -- the corpus does not exercise the distinction and PrintXMILE
// reads naming state from the populated Model. isAsSectors: XMILE input is
// always emitted in the single-<model> sector form, because the module
// decomposition `false` would select emits sibling <model> elements that this
// tool's own reader rejects.
XMUTIL_EXPORT char *convert_xmile_to_xmile(const char *source, uint32_t len, const char *fileName, int isLongName,
                                           bool isAsSectors);

// returns NULL on error or a string containing Vensim .mdl that the caller now
// owns. isLongName is accepted for API symmetry; see convert_xmile_to_xmile.
XMUTIL_EXPORT char *convert_xmile_to_mdl(const char *source, uint32_t len, const char *fileName, int isLongName);
}

// utility functions
std::string StringFromDouble(double val);
// The shortest decimal string that parses back to exactly `val`
// (std::to_chars). StringFromDouble's "%g" (~6 significant digits) is fine for
// sketch coordinates but lossy for values like lookup-table samples, where a
// re-parse must recover the identical double. Shared by the XMILE writer's
// lookup samples, FormatMDLNumber's non-integer tail, and
// ExpressionNumber::OutputComputable so all three emit identical text.
std::string ShortestDouble(double val);
std::string SpaceToUnderBar(const std::string &s);
std::string QuotedSpaceToUnderBar(const std::string &s);
bool StringMatch(const std::string &f, const std::string &s);  // asciii only;
double AngleFromPoints(double startx, double starty, double pointx, double pointy, double endx, double endy);
#endif
