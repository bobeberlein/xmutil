// Emscripten wrapper for XMUtil
// Exposes convert_mdl_to_xmile as a JavaScript-callable function

#include <string>
#include <cstdlib>

extern "C" {
    char *convert_mdl_to_xmile(const char *mdlSource, uint32_t mdlSourceLen, const char *fileName, bool isCompact, int isLongName, bool isAsSectors);
}

#ifdef __EMSCRIPTEN__
#include <emscripten/bind.h>
// JavaScript-friendly wrapper that takes a string and returns a string
std::string convertMdlToXmile(const std::string& mdlContent, bool isCompact = false, int isLongName = 0, bool isAsSectors = false) {
    const char* mdlSource = mdlContent.c_str();
    uint32_t mdlSourceLen = mdlContent.length();
    const char* fileName = "input.mdl";

    char* result = convert_mdl_to_xmile(mdlSource, mdlSourceLen, fileName, isCompact, isLongName, isAsSectors);

    if (result == nullptr) {
        return "";  // Return empty string on error
    }

    std::string xmileContent(result);
    free(result);  // Free the C string allocated by convert_mdl_to_xmile

    return xmileContent;
}

// Bind the function to JavaScript with optional parameters
EMSCRIPTEN_BINDINGS(xmutil_module) {
    emscripten::function("convertMdlToXmile",
        emscripten::optional_override([](const std::string& mdlContent) {
            return convertMdlToXmile(mdlContent);
        }),
        emscripten::allow_raw_pointers());
    emscripten::function("convertMdlToXmile",
        emscripten::optional_override([](const std::string& mdlContent, bool isCompact) {
            return convertMdlToXmile(mdlContent, isCompact);
        }),
        emscripten::allow_raw_pointers());
    emscripten::function("convertMdlToXmile",
        emscripten::optional_override([](const std::string& mdlContent, bool isCompact, int isLongName) {
            return convertMdlToXmile(mdlContent, isCompact, isLongName);
        }),
        emscripten::allow_raw_pointers());
    emscripten::function("convertMdlToXmile", &convertMdlToXmile, emscripten::allow_raw_pointers());
}
#endif
