// Unicode.cpp : Wraps unicode-specific functionality

#include <cstring>

#include "Unicode.h"

#include "unicode/utypes.h"
#include "unicode/ustring.h"
#include "unicode/ucasemap.h"


static UCaseMap *GlobalUCaseMap;
bool OpenUnicode() {
    UErrorCode ec = U_ZERO_ERROR;
    GlobalUCaseMap = ucasemap_open("en",0,&ec) ;
    return GlobalUCaseMap != nullptr;
}

void CloseUnicode() {
   ucasemap_close(GlobalUCaseMap) ;
}

char *utf8ToLower(const char *src, size_t srcLen) {
    char *dst = new char[srcLen+2];
    memset(dst, 0, srcLen+2);

    UErrorCode ec = U_ZERO_ERROR;
    ucasemap_utf8ToLower(GlobalUCaseMap, dst, srcLen+1, src, srcLen, &ec);
    if (ec != U_ZERO_ERROR) {
      delete[] dst;
      return nullptr;
    }

    return dst;
}
