#!/bin/bash
set -e

PKG_VERSION=54.1
SDK_VERSION=10.12
OSX_VERSION=10.7

ARCHIVE_NAME="icu4c-$(echo $PKG_VERSION | sed -e 's/\./_/')-src.zip"
DOWNLOAD_URL="http://download.icu-project.org/files/icu4c/${PKG_VERSION}/${ARCHIVE_NAME}"
PKG_NAME='icu'
LIB_NAME="lib${PKG_NAME}.a"

WORK_PATH="$PWD/build_icu_mac"
ARCHS="x86_64"

mkdir -p ${WORK_PATH}/src
mkdir -p ${WORK_PATH}/lib
mkdir -p ${WORK_PATH}/include/${PKG_NAME}
mkdir -p ${WORK_PATH}/objs
mkdir -p ${WORK_PATH}/local

pushd ${WORK_PATH}/src > /dev/null
if [ ! -e ${ARCHIVE_NAME} ]; then
    echo "Downloading ${ARCHIVE_NAME}"
    curl -O -L ${DOWNLOAD_URL}
    if [ ! -d icu ]; then
	unzip -a ${ARCHIVE_NAME} > /dev/null
    chmod a+x ${WORK_PATH}/src/icu/source/configure
	# this doesn't apply cleanly
	# https://chromiumcodereview.appspot.com/10692124/
	#(cd icu; patch -p0 <${WORK_PATH}/../ios_timezone.patch)
    fi
else
    echo "Using ${ARCHIVE_NAME}"
fi

echo "starting cross-build"
mkdir -p ${WORK_PATH}/objs/cross-build
pushd ${WORK_PATH}/objs/cross-build >/dev/null
if [ ! -e Makefile ]; then
    ../../src/icu/source/configure --disable-dyload --enable-static --disable-shared --enable-release
fi
make -j4
popd >/dev/null
echo "finshed cross-build"

for ARCH in ${ARCHS}
do
    PLATFORM="MacOSX"
    CONFIG_FLAGS=""
    export DEV_ROOT="/Applications/Xcode.app/Contents/Developer/Platforms/${PLATFORM}.platform/Developer"
    export SDK_ROOT="${DEV_ROOT}/SDKs/${PLATFORM}${SDK_VERSION}.sdk"
    BUILD_PATH="${WORK_PATH}/objs/${PLATFORM}${SDK_VERSION}-${ARCH}.sdk"

    export CC="clang -isysroot ${SDK_ROOT} -mmacosx-version-min=${OSX_VERSION}"
    export CXX="clang++ -stdlib=libc++ -isysroot ${SDK_ROOT} -mmacosx-version-min=${OSX_VERSION}"
    export LDFLAGS="-isysroot ${SDK_ROOT}"
    export CFLAGS="-I${WORK_PATH}/src/icu/source/tools/tzcode"
    export CXXFLAGS="$CFLAGS"

    echo "Building ${PKG_NAME} for ${PLATFORM} ${SDK_VERSION} ${ARCH} ..."
    
    mkdir -p ${BUILD_PATH}

    LOG="${BUILD_PATH}/build-${PKG_NAME}-${PKG_VERSION}.log"

    pushd ${BUILD_PATH} > /dev/null
    ../../src/icu/source/configure $CONFIG_FLAGS --disable-dyload --enable-static --disable-shared --enable-release --prefix=${WORK_PATH}/local
    make  -j6 # >> "${LOG}" 2>&1
    make install
    popd > /dev/null
done

echo "Creating universal  library..."
for i in `ls ${WORK_PATH}/objs/MacOSX${SDK_VERSION}-x86_64.sdk/lib/*.a`; do
    lib=`basename $i`
    echo "lib: $lib"
    rm -f ${WORK_PATH}/lib/$lib
    lipo -create ${WORK_PATH}/objs/MacOSX${SDK_VERSION}-x86_64.sdk/lib/$lib -output ${WORK_PATH}/lib/$lib
done
echo "Build ${LIB_NAME} done."

mkdir -p ${WORK_PATH}/../../../third_party/include
mkdir -p ${WORK_PATH}/../../../third_party/mac/lib

cp -a ${WORK_PATH}/local/include/* ${WORK_PATH}/../../../third_party/include
cp -a ${WORK_PATH}/lib/* ${WORK_PATH}/../../../third_party/mac/lib
