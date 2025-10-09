// Unicode_stub.cpp : Simple stub implementation for WASM builds
// This avoids the need for full ICU library linking

#include <algorithm>
#include <cctype>
#include <cstring>

#include "Unicode.h"

bool OpenUnicode() {
    // Stub - always succeeds
    return true;
}

void CloseUnicode() {
    // Stub - nothing to close
}

char *utf8ToLower(const char *src, size_t srcLen) {
    // Simple ASCII lowercase conversion
    // For full Unicode support, would need ICU
    char *dst = new char[srcLen + 1];

    for (size_t i = 0; i < srcLen; i++) {
        dst[i] = std::tolower(static_cast<unsigned char>(src[i]));
    }
    dst[srcLen] = '\0';

    return dst;
}
