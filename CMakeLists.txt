cmake_minimum_required(VERSION 3.16)

# The Intel Mac build must run on macOS 10.15 (matching the ICU build in
# third_party/build/icu_mac.bash), so the code avoids libc++ features
# introduced later (floating-point std::to_chars is 13.3+). arm64 builds are
# clamped to 11.0 by the toolchain.
# Must be set before project().
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum macOS version")

project(XMUtil LANGUAGES C CXX)

option(XMUTIL_WITH_UI "Build the Qt UI into XMUtil" OFF)
option(XMUTIL_BUILD_TESTS "Build the xmutil_test harness" ON)

get_property(xmutil_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(NOT xmutil_multi_config AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(xmutil_common_sources
  src/XMUtil.h
  src/XMUtil.cpp
  src/Log.h
  src/Log.cpp
  src/Model.h
  src/Model.cpp
  src/ContextInfo.h
  src/ContextInfo.cpp

  src/Xmile/XMILEGenerator.h
  src/Xmile/XMILEGenerator.cpp
  src/Xmile/XmileReader.h
  src/Xmile/XmileReader.cpp
  src/Xmile/XmileView.h
  src/Xmile/XmileView.cpp
  src/Xmile/XmileFunctions.h
  src/Xmile/XmileFunctions.cpp
  src/Xmile/XmileEqLex.h
  src/Xmile/XmileEqLex.cpp
  src/Xmile/XmileEqYacc.tab.cpp
  src/Xmile/XmileEqYacc.tab.hpp
  src/Xmile/XmileParseFunctions.h
  src/Xmile/XmileParseFunctions.cpp

  src/Mdl/MDLFormat.h
  src/Mdl/MDLFormat.cpp
  src/Mdl/MDLGenerator.h
  src/Mdl/MDLGenerator.cpp

  src/Vensim/VensimLex.h
  src/Vensim/VensimLex.cpp
  src/Vensim/VensimParse.h
  src/Vensim/VensimParse.cpp
  src/Vensim/VensimParseFunctions.h
  src/Vensim/VensimParseFunctions.cpp
  src/Vensim/VensimView.h
  src/Vensim/VensimView.cpp
  src/Vensim/VYacc.tab.cpp
  src/Vensim/VYacc.tab.hpp

  src/Dynamo/DynamoLex.h
  src/Dynamo/DynamoLex.cpp
  src/Dynamo/DynamoFunction.h
  src/Dynamo/DynamoFunction.cpp
  src/Dynamo/DynamoParse.h
  src/Dynamo/DynamoParse.cpp
  src/Dynamo/DynamoParseFunctions.h
  src/Dynamo/DynamoParseFunctions.cpp
  src/Dynamo/DYacc.tab.cpp
  src/Dynamo/DYacc.tab.hpp

  src/Symbol/Variable.h
  src/Symbol/Variable.cpp
  src/Symbol/Units.h
  src/Symbol/Units.cpp
  src/Symbol/UnitExpression.h
  src/Symbol/UnitExpression.cpp
  src/Symbol/SymbolTableBase.h
  src/Symbol/SymbolTableBase.cpp
  src/Symbol/SymbolNameSpace.h
  src/Symbol/SymbolNameSpace.cpp
  src/Symbol/SymbolListList.h
  src/Symbol/SymbolListList.cpp
  src/Symbol/SymbolList.h
  src/Symbol/SymbolList.cpp
  src/Symbol/Symbol.h
  src/Symbol/Symbol.cpp
  src/Symbol/Parse.h
  src/Symbol/NotUsed_SymAllocList.h
  src/Symbol/NotUsed_SymAllocList.cpp
  src/Symbol/LeftHandSide.h
  src/Symbol/LeftHandSide.cpp
  src/Symbol/ExpressionList.h
  src/Symbol/ExpressionList.cpp
  src/Symbol/Expression.h
  src/Symbol/Expression.cpp
  src/Symbol/Equation.h
  src/Symbol/Equation.cpp

  src/Function/Function.h
  src/Function/Function.cpp
  src/Function/Level.h
  src/Function/Level.cpp
  src/Function/State.h
  src/Function/State.cpp
  src/Function/TableFunction.h
  src/Function/TableFunction.cpp
)

# ---------------------------------------------------------------------------
# WebAssembly: configure with `emcmake cmake --preset wasm`. Only the wasm
# module is defined in that tree; the native CLI and tests need ICU.
# ---------------------------------------------------------------------------
if(EMSCRIPTEN)
  # The wasm build swaps ICU for a stub (src/Unicode_stub.cpp) and compiles
  # tinyxml2 from the source tree third_party/all_mac.sh checks out.
  set(xmutil_wasm_sources ${xmutil_common_sources})
  list(APPEND xmutil_wasm_sources
    src/emscripten_wrapper.cpp
    src/Unicode_stub.cpp
    third_party/build/tinyxml2/tinyxml2.cpp
  )
  add_executable(XMUtil_wasm ${xmutil_wasm_sources})
  set_target_properties(XMUtil_wasm PROPERTIES OUTPUT_NAME xmutil SUFFIX ".js")
  target_include_directories(XMUtil_wasm PRIVATE src third_party/include)
  target_compile_options(XMUtil_wasm PRIVATE -fexceptions -O2)
  target_link_options(XMUtil_wasm PRIVATE
    -fexceptions
    -O2
    --bind
    "SHELL:-s WASM=1"
    "SHELL:-s MODULARIZE=1"
    "SHELL:-s EXPORT_NAME=createXMUtilModule"
    "SHELL:-s EXPORTED_RUNTIME_METHODS=[ccall,cwrap]"
    "SHELL:-s ALLOW_MEMORY_GROWTH=1"
    "SHELL:-s MAXIMUM_MEMORY=512MB"
  )
  return()
endif()

list(APPEND xmutil_common_sources src/Unicode.h src/Unicode.cpp)

# ---------------------------------------------------------------------------
# Platform dependencies (ICU + tinyxml2) and shared compile settings, carried
# by an interface target so XMUtil and xmutil_test stay in lockstep.
# ---------------------------------------------------------------------------
add_library(xmutil_platform INTERFACE)
target_include_directories(xmutil_platform INTERFACE src)
target_compile_definitions(xmutil_platform INTERFACE
  $<$<CONFIG:Debug>:_DEBUG_XMUTIL>
)

set(xmutil_platform_sources)
set(xmutil_runtime_dlls)

if(WIN32)
  # ICU is provisioned by hand into third_party/win (see README.md). Nothing
  # provisions a tinyxml2.lib, so the amalgamated source next to the header is
  # compiled in instead.
  set(xmutil_win_lib "${CMAKE_CURRENT_SOURCE_DIR}/third_party/win/lib")
  target_include_directories(xmutil_platform INTERFACE third_party/include)
  target_compile_definitions(xmutil_platform INTERFACE WIN32=1)
  target_link_libraries(xmutil_platform INTERFACE
    "${xmutil_win_lib}/icudt.lib"
    "${xmutil_win_lib}/icuin.lib"
    "${xmutil_win_lib}/icuio.lib"
    "${xmutil_win_lib}/icutu.lib"
    "${xmutil_win_lib}/icuuc.lib"
  )
  if(MSVC)
    # /utf-8: sources carry UTF-8 literals; without it MSVC decodes them with
    # the machine's ANSI codepage.
    target_compile_options(xmutil_platform INTERFACE /EHsc /MP /utf-8)
  endif()
  list(APPEND xmutil_platform_sources third_party/include/tinyxml2.cpp)
  # ICU's DLL names carry its major version, so glob rather than pin one.
  file(GLOB xmutil_runtime_dlls "${xmutil_win_lib}/dlls/*.dll")
elseif(APPLE)
  # Static ICU and tinyxml2 built by third_party/all_mac.sh, for the single
  # architecture being built. Override the directory to cross-build (e.g. an
  # Intel build on Apple silicon against separately built x86_64 libs).
  set(XMUTIL_MAC_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/mac/lib"
    CACHE PATH "Directory holding the static ICU and tinyxml2 libraries")
  set(xmutil_mac_lib "${XMUTIL_MAC_LIB_DIR}")
  target_include_directories(xmutil_platform INTERFACE third_party/include)
  target_compile_definitions(xmutil_platform INTERFACE HAVE_SYS_MALLOC_H)
  target_compile_options(xmutil_platform INTERFACE
    -Wno-tautological-compare
    -Wno-parentheses-equality
    -Werror=return-type
    -Winconsistent-missing-override
    -Wno-unused-parameter
    -Wno-invalid-source-encoding
  )
  target_link_libraries(xmutil_platform INTERFACE
    "${xmutil_mac_lib}/libtinyxml2.a"
    "${xmutil_mac_lib}/libicudata.a"
    "${xmutil_mac_lib}/libicui18n.a"
    "${xmutil_mac_lib}/libicuio.a"
    "${xmutil_mac_lib}/libicule.a"
    "${xmutil_mac_lib}/libiculx.a"
    "${xmutil_mac_lib}/libicutu.a"
    "${xmutil_mac_lib}/libicuuc.a"
  )
else()
  # Linux: system ICU and tinyxml2, matching upstream.
  find_package(ICU REQUIRED COMPONENTS uc)
  find_package(tinyxml2 CONFIG QUIET)
  if(TARGET tinyxml2::tinyxml2)
    set(xmutil_tinyxml2 tinyxml2::tinyxml2)
  else()
    find_path(TINYXML2_INCLUDE_DIR tinyxml2.h REQUIRED)
    find_library(TINYXML2_LIBRARY tinyxml2 REQUIRED)
    add_library(xmutil_tinyxml2 INTERFACE)
    target_include_directories(xmutil_tinyxml2 INTERFACE "${TINYXML2_INCLUDE_DIR}")
    target_link_libraries(xmutil_tinyxml2 INTERFACE "${TINYXML2_LIBRARY}")
    set(xmutil_tinyxml2 xmutil_tinyxml2)
  endif()
  target_compile_definitions(xmutil_platform INTERFACE linux)
  target_compile_options(xmutil_platform INTERFACE -Wall -Wextra -Wno-unused-parameter)
  target_link_libraries(xmutil_platform INTERFACE ICU::uc ${xmutil_tinyxml2})
  set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

function(xmutil_copy_runtime_dlls target)
  if(xmutil_runtime_dlls)
    add_custom_command(TARGET ${target} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${xmutil_runtime_dlls} "$<TARGET_FILE_DIR:${target}>"
      VERBATIM)
  endif()
endfunction()

# ---------------------------------------------------------------------------
# XMUtil: the CLI (optionally with the Qt UI).
# ---------------------------------------------------------------------------
add_executable(XMUtil
  src/Main.cpp
  # CLI-only: the wasm and library entry points never derive an output
  # filename, so this stays out of xmutil_common_sources.
  src/OutputPath.h
  src/OutputPath.cpp
  ${xmutil_common_sources}
  ${xmutil_platform_sources}
)
target_link_libraries(XMUtil PRIVATE xmutil_platform)
xmutil_copy_runtime_dlls(XMUtil)

if(XMUTIL_WITH_UI)
  # Point CMAKE_PREFIX_PATH (or Qt5_DIR) at the Qt install, e.g.
  # -DCMAKE_PREFIX_PATH=$HOME/Qt5.15.4/5.15.4/clang_arm64
  find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets)
  target_sources(XMUtil PRIVATE
    src/UI/Main_Window.h
    src/UI/Main_Window.cpp
    src/UI/resources/main_window.ui
  )
  set_target_properties(XMUtil PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTOUIC_SEARCH_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/src/UI/resources"
    MACOSX_BUNDLE ON
    WIN32_EXECUTABLE ON
  )
  target_compile_definitions(XMUtil PRIVATE WITH_UI)
  target_link_libraries(XMUtil PRIVATE Qt5::Core Qt5::Gui Qt5::Widgets)
endif()

# ---------------------------------------------------------------------------
# xmutil_test: links the same engine sources as XMUtil but swaps Main.cpp for
# the self-registering harness in test/TestHarness.cpp.
# ---------------------------------------------------------------------------
if(XMUTIL_BUILD_TESTS)
  add_executable(xmutil_test
    ${xmutil_common_sources}
    ${xmutil_platform_sources}
    # Main.cpp is replaced by the harness, so the CLI-only sources it depends
    # on have to be listed here too.
    src/OutputPath.h
    src/OutputPath.cpp
    test/TestHarness.cpp
    test/UnicodeTest.cpp
    test/OutputPathTest.cpp
    test/ShortestDoubleTest.cpp
    test/mdl/ModelComparator.cpp
    test/mdl/RoundTrip.cpp
    test/mdl/MDLGeneratorTest.cpp
    test/mdl/MDLFormatTest.cpp
    test/mdl/WalkerTest.cpp
    test/mdl/ModelComparatorTest.cpp
    test/mdl/EquationRoundTripTest.cpp
    test/mdl/ControlRoundTripTest.cpp
    test/mdl/SketchRoundTripTest.cpp
    test/mdl/MacroRoundTripTest.cpp
    test/mdl/WriterBugRegressionTest.cpp
    test/mdl/GroupNestingTest.cpp
    test/mdl/CorpusRoundTripTest.cpp
    test/mdl/CEntryTest.cpp
    test/mdl/DynamoToMdlTest.cpp
    test/mdl/CrossViewFlowTest.cpp
    test/xmile/RoundTrip.cpp
    test/xmile/BasicSmokeTest.cpp
    test/xmile/XmileFunctionsTest.cpp
    test/xmile/EquationParseTest.cpp
    test/xmile/AuxRoundTripTest.cpp
    test/xmile/SimSpecsRoundTripTest.cpp
    test/xmile/StockFlowRoundTripTest.cpp
    test/xmile/ArrayRoundTripTest.cpp
    test/xmile/LookupRoundTripTest.cpp
    test/xmile/FreeTextSanitizeRoundTripTest.cpp
    test/xmile/ModelUnitsRoundTripTest.cpp
    test/xmile/ExtrapolateRoundTripTest.cpp
    test/xmile/ViewRoundTripTest.cpp
    test/xmile/GroupRoundTripTest.cpp
    test/xmile/SingleModelNormalizationTest.cpp
    test/xmile/CorpusRoundTripTest.cpp
    test/xmile/XmileCorpusTest.cpp
    test/xmile/FixpointTest.cpp
    test/xmile/ErrorRejectTest.cpp
    test/xmile/BuiltinNameCollisionTest.cpp
    test/xmile/PiKeywordTest.cpp
    test/xmile/ReaderLifetimeTest.cpp
    test/xmile/DiagnosticsTest.cpp
    test/xmile/CEntryTest.cpp
    test/xmile/MdlXmileByteIdentityTest.cpp
  )
  target_link_libraries(xmutil_test PRIVATE xmutil_platform)
  # The corpus tests join the repo root with relative fixture paths so they
  # locate on-disk fixtures regardless of the test binary's CWD.
  target_compile_definitions(xmutil_test PRIVATE
    XMUTIL_SRC_ROOT="${CMAKE_CURRENT_SOURCE_DIR}"
  )
  xmutil_copy_runtime_dlls(xmutil_test)

  enable_testing()
  add_test(NAME xmutil_test COMMAND xmutil_test)

  find_program(BASH_EXECUTABLE bash)
  if(BASH_EXECUTABLE AND NOT XMUTIL_WITH_UI)
    add_test(NAME cli_roundtrip
      COMMAND ${BASH_EXECUTABLE} test/cli_roundtrip.sh $<TARGET_FILE:XMUtil>
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    add_test(NAME cli_xmile_roundtrip
      COMMAND ${BASH_EXECUTABLE} test/cli_xmile_roundtrip.sh $<TARGET_FILE:XMUtil>
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  endif()
endif()
