#!/bin/bash
# Script to build WASM target - filters out .h files and compiles with em++

OUTPUT_DIR="$1"
shift

# Filter out header files and .y files, keep only .cpp files
CPP_FILES=()
for file in "$@"; do
    case "$file" in
        *.cpp)
            CPP_FILES+=("$file")
            ;;
    esac
done

# Build with em++ - exclude Unicode.cpp and use stub instead
em++ \
    -std=c++17 \
    -fexceptions \
    -Isrc \
    -Ithird_party/include \
    $(echo "${CPP_FILES[@]}" | tr ' ' '\n' | grep -v "Unicode\.cpp") \
    src/Unicode_stub.cpp \
    third_party/build/tinyxml2/tinyxml2.cpp \
    -o "${OUTPUT_DIR}/xmutil.js" \
    -s WASM=1 \
    -s MODULARIZE=1 \
    -s "EXPORT_NAME=createXMUtilModule" \
    -s "EXPORTED_RUNTIME_METHODS=[ccall,cwrap]" \
    --bind \
    -s ALLOW_MEMORY_GROWTH=1 \
    -s "MAXIMUM_MEMORY=512MB" \
    -O2
