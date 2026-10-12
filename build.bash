#!/usr/bin/env bash
#
# Builds the XMUtil release programs and runs the tests.
#
#   macOS: out/mac-arm/XMUtil   (Apple silicon, macOS 11.0+)
#          out/mac-intel/XMUtil (Intel, macOS 10.15+)
#          The static ICU and tinyxml2 they link live in
#          third_party/mac/lib-<arch>; any that are missing are built first
#          (downloads and intermediate builds go to out/third_party).
#   Linux: out/Release/XMUtil, native, against the system ICU and tinyxml2.
#
# Usage: ./build.bash [--skip-tests]

set -euo pipefail

cd "$(dirname "$0")"
ROOT=$PWD

ICU_VERSION=54.2 # must match the headers in third_party/include/unicode
TINYXML2_VERSION=8.0.0
WORK=$ROOT/out/third_party

RUN_TESTS=yes
for arg in "$@"; do
  case $arg in
  --skip-tests) RUN_TESTS=no ;;
  -h | --help)
    sed -n '3,15p' "$0"
    exit 0
    ;;
  *)
    echo "unknown option: $arg" >&2
    exit 2
    ;;
  esac
done

say() { printf '\n==> %s\n' "$*"; }
die() {
  echo "error: $*" >&2
  exit 1
}
need() { command -v "$1" >/dev/null 2>&1 || die "$1 is required but not on PATH. $2"; }

# --------------------------------------------------------------------------
# macOS third-party libraries
# --------------------------------------------------------------------------
MAC_LIBS="libicudata libicui18n libicuio libicule libiculx libicutu libicuuc libtinyxml2"

min_macos() { if [ "$1" = arm64 ]; then echo 11.0; else echo 10.15; fi; }
host_triple() { if [ "$1" = arm64 ]; then echo aarch64-apple-darwin; else echo x86_64-apple-darwin; fi; }

fetch_tinyxml2() {
  TINYXML2_SRC=$WORK/tinyxml2-$TINYXML2_VERSION
  if [ ! -f "$TINYXML2_SRC/tinyxml2.cpp" ]; then
    say "Downloading tinyxml2 $TINYXML2_VERSION"
    mkdir -p "$TINYXML2_SRC"
    for f in tinyxml2.cpp tinyxml2.h; do
      curl -fsSL -o "$TINYXML2_SRC/$f" \
        "https://raw.githubusercontent.com/leethomason/tinyxml2/$TINYXML2_VERSION/$f"
    done
  fi
}

fetch_icu() {
  ICU_SRC=$WORK/icu-$ICU_VERSION/icu/source
  if [ ! -x "$ICU_SRC/configure" ]; then
    local tag=${ICU_VERSION//./-} file=icu4c-${ICU_VERSION//./_}-src.tgz
    say "Downloading ICU $ICU_VERSION"
    mkdir -p "$WORK/icu-$ICU_VERSION"
    curl -fsSL -o "$WORK/$file" \
      "https://github.com/unicode-org/icu/releases/download/release-$tag/$file"
    tar -xzf "$WORK/$file" -C "$WORK/icu-$ICU_VERSION"
  fi
}

# Builds static ICU for one architecture into $WORK/icu-install-<arch>. ICU's
# build runs tools it has just compiled, so a non-native architecture is
# cross-built against a native build's tools (--with-cross-build).
build_icu() {
  local arch=$1 native
  native=$(uname -m)
  local build=$WORK/icu-build-$arch install=$WORK/icu-install-$arch
  [ -f "$install/lib/libicuuc.a" ] && return
  fetch_icu

  local cross=()
  if [ "$arch" != "$native" ]; then
    build_icu "$native"
    cross=(--host="$(host_triple "$arch")" --with-cross-build="$WORK/icu-build-$native")
  fi

  say "Building ICU $ICU_VERSION for $arch (macOS $(min_macos "$arch")+)"
  rm -rf "$build" && mkdir -p "$build"
  local min
  min=$(min_macos "$arch")
  (
    cd "$build"
    export CC="clang -arch $arch -mmacosx-version-min=$min"
    export CXX="clang++ -arch $arch -stdlib=libc++ -mmacosx-version-min=$min"
    export CFLAGS="-I$ICU_SRC/tools/tzcode" CXXFLAGS="-I$ICU_SRC/tools/tzcode"
    "$ICU_SRC/configure" ${cross[@]+"${cross[@]}"} --disable-dyload --enable-static --disable-shared \
      --enable-release --disable-tests --disable-samples --prefix="$install" >configure.log 2>&1 ||
      die "ICU configure failed for $arch; see $build/configure.log"
    make -j"$(sysctl -n hw.ncpu)" >make.log 2>&1 || die "ICU build failed for $arch; see $build/make.log"
    make install >install.log 2>&1 || die "ICU install failed for $arch; see $build/install.log"
  )
}

mac_libs() {
  local arch=$1 dir=$ROOT/third_party/mac/lib-$1 missing=no lib
  for lib in $MAC_LIBS; do [ -f "$dir/$lib.a" ] || missing=yes; done
  if [ $missing = yes ]; then
    build_icu "$arch"
    fetch_tinyxml2
    say "Building tinyxml2 $TINYXML2_VERSION for $arch"
    mkdir -p "$WORK/tinyxml2-$arch" "$dir"
    clang++ -arch "$arch" -mmacosx-version-min="$(min_macos "$arch")" -std=c++17 -O2 \
      -c "$TINYXML2_SRC/tinyxml2.cpp" -o "$WORK/tinyxml2-$arch/tinyxml2.o"
    rm -f "$WORK/tinyxml2-$arch/libtinyxml2.a"
    ar rcs "$WORK/tinyxml2-$arch/libtinyxml2.a" "$WORK/tinyxml2-$arch/tinyxml2.o"
    cp "$WORK/tinyxml2-$arch/libtinyxml2.a" "$dir/"
    for lib in $MAC_LIBS; do
      [ $lib = libtinyxml2 ] || cp "$WORK/icu-install-$arch/lib/$lib.a" "$dir/"
    done
  fi

  # The headers are shared by both architectures.
  local inc=$ROOT/third_party/include
  mkdir -p "$inc"
  if [ ! -f "$inc/unicode/uvernum.h" ]; then
    build_icu "$arch"
    cp -R "$WORK/icu-install-$arch/include/unicode" "$inc/"
  fi
  if [ ! -f "$inc/tinyxml2.h" ]; then
    fetch_tinyxml2
    cp "$TINYXML2_SRC/tinyxml2.h" "$inc/"
  fi
  grep -q "U_ICU_VERSION \"$ICU_VERSION\"" "$inc/unicode/uvernum.h" ||
    die "third_party/include/unicode is not ICU $ICU_VERSION; remove it to have it replaced"
}

# Runs a binary of this architecture here? arm64 needs Apple silicon; x86_64
# runs natively on Intel or under Rosetta on Apple silicon.
can_run() {
  [ "$1" = "$(uname -m)" ] && return 0
  [ "$1" = x86_64 ] && arch -x86_64 /usr/bin/true >/dev/null 2>&1
}

build_preset() {
  local preset=$1
  shift
  say "Configuring $preset"
  cmake --preset "$preset" "$@" >/dev/null
  say "Building $preset"
  cmake --build --preset "$preset"
}

# --------------------------------------------------------------------------
case "$(uname -s)" in
Darwin)
  need cmake "Install it with 'brew install cmake'."
  need ninja "Install it with 'brew install ninja'."
  need clang++ "Install the Xcode command line tools: xcode-select --install"

  built=()
  for arch in arm64 x86_64; do
    if [ $arch = arm64 ]; then preset=mac-arm; else preset=mac-intel; fi
    mac_libs $arch
    build_preset $preset -DXMUTIL_MAC_LIB_DIR="$ROOT/third_party/mac/lib-$arch"
    if [ $RUN_TESTS = yes ]; then
      if can_run $arch; then
        say "Testing $preset"
        ctest --preset $preset
      else
        say "Skipping $preset tests: $arch binaries cannot run on this Mac"
      fi
    fi
    built+=("out/$preset/XMUtil")
  done
  ;;
Linux)
  hint="Fedora: sudo dnf install cmake ninja-build gcc-c++ libicu-devel tinyxml2-devel
  Debian/Ubuntu: sudo apt install cmake ninja-build g++ libicu-dev libtinyxml2-dev
  Alpine: apk add cmake ninja build-base icu-dev tinyxml2-dev"
  need cmake "$hint"
  need ninja "$hint"
  need c++ "$hint"
  # Probe the system ICU and tinyxml2 headers the same way the compiler will.
  echo '#include <unicode/uchar.h>
#include <tinyxml2.h>' | c++ -x c++ -fsyntax-only - 2>/dev/null ||
    die "the ICU and tinyxml2 development packages are missing. Install them with
  $hint"

  build_preset release
  if [ $RUN_TESTS = yes ]; then
    say "Testing release"
    ctest --preset release
  fi
  built=(out/Release/XMUtil)
  ;;
*)
  die "unsupported platform $(uname -s); on Windows run build.bat"
  ;;
esac

say "Done"
for b in "${built[@]}"; do echo "  $ROOT/$b"; done
