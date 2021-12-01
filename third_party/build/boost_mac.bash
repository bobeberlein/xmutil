#!/bin/bash

#===============================================================================
# Filename:  boost.sh
# Author:    Pete Goodliffe
# Copyright: (c) Copyright 2009 Pete Goodliffe
# Licence:   Please feel free to use this, with attribution
#===============================================================================
#
# Builds a Boost framework for the iPhone.
# Creates a set of universal libraries that can be used on an iPhone and in the
# iPhone simulator. Then creates a pseudo-framework to make using boost in Xcode
# less painful.
#
# To configure the script, define:
#    BOOST_LIBS:        which libraries to build
#    IPHONE_SDKVERSION: iPhone SDK version (e.g. 5.1)
#
# Then go get the source tar.bz of the boost you want to build, shove it in the
# same directory as this script, and run "./boost.sh". Grab a cuppa. And voila.
#===============================================================================

ARCH_GENERIC=x86
ARCH=x86_64

for arg in "$@"
do
    case $arg in
    --mac-arm)
        ARCH_GENERIC=arm
        ARCH=arm64
    ;;
    --mac-x86_64)
        ARCH_GENERIC=x86
        ARCH=x86_64
    ;;
    esac
done

PKG_NAME='boost'
PKG_VERSION_A=1.76.0
PKG_VERSION_B=boost_1_76_0
ARCHIVE_NAME="${PKG_VERSION_B}.tar.gz"
DOWNLOAD_URL="http://downloads.sourceforge.net/project/boost/boost/${PKG_VERSION_A}/${ARCHIVE_NAME}"

: ${BOOST_LIBS:="thread filesystem system date_time random chrono program_options iostreams"}
: ${OSX_SDKVERSION:=12.0}
: ${OSX_VERSION:=10.15}
: ${XCODE_ROOT:=`xcode-select -print-path`}
: ${EXTRA_CPPFLAGS:="-DBOOST_AC_USE_PTHREADS -DBOOST_SP_USE_PTHREADS -std=c++11 -stdlib=libc++"}

# The EXTRA_CPPFLAGS definition works around a thread race issue in
# shared_ptr. I encountered this historically and have not verified that
# the fix is no longer required. Without using the posix thread primitives
# an invalid compare-and-swap ARM instruction (non-thread-safe) was used for the
# shared_ptr use count causing nasty and subtle bugs.
#
# Should perhaps also consider/use instead: -BOOST_SP_USE_PTHREADS

: ${SRCDIR:=`pwd`}
: ${OSXBUILDDIR:=`pwd`/osx/build}
: ${COMPILER:="clang++"}
: ${PREFIXDIR:=`pwd`/osx/prefix}

BOOST_SRC=$SRCDIR/build_boost_mac
#===============================================================================

#===============================================================================
# Functions
#===============================================================================

abort()
{
    echo
    echo "Aborted: $@"
    exit 1
}

doneSection()
{
    echo
    echo "    ================================================================="
    echo "    Done"
    echo
}

#===============================================================================

cleanEverythingReadyToStart()
{
    echo Cleaning everything before we start to build...
    rm -rf osx-build
    rm -rf $OSXBUILDDIR
    rm -rf $PREFIXDIR
    doneSection
}

#===============================================================================
updateBoost()
{
    echo Updating boost into $BOOST_SRC...

    OSX_SYSROOT="/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX${OSX_SDKVERSION}.sdk"

    if [ ! -e $BOOST_SRC ]; then
        echo "Downloading ${DOWNLOAD_URL}"
        curl -L -O ${DOWNLOAD_URL}
        tar -xJf ${ARCHIVE_NAME} > /dev/null
        mv ${PKG_VERSION_B} "build_boost_mac"

        cat >> $BOOST_SRC/tools/build/v2/user-config.jam <<EOF
using darwin : ${OSX_SDKVERSION}~mac
   : $XCODE_ROOT/Toolchains/XcodeDefault.xctoolchain/usr/bin/$COMPILER -arch ${ARCH} -fvisibility=hidden -fvisibility-inlines-hidden $EXTRA_CPPFLAGS -isysroot $OSX_SYSROOT -mmacosx-version-min=$OSX_VERSION
   : <striper> <root>$XCODE_ROOT/Platforms/MacOSX.platform/Developer
   : <address-model>64 <architecture>${ARCH_GENERIC} <target-os>darwin
   ;
EOF
    else
        echo "Using ${ARCHIVE_NAME}"
    fi

    doneSection
}

#===============================================================================

bootstrapBoost()
{
    cd $BOOST_SRC
    BOOST_LIBS_COMMA=$(echo $BOOST_LIBS | sed -e "s/ /,/g")
    echo "Bootstrapping (with libs $BOOST_LIBS_COMMA)"
    ./bootstrap.sh --with-libraries=$BOOST_LIBS_COMMA
    doneSection
}

#===============================================================================

buildBoostForOSX()
{
    cd $BOOST_SRC

    ./b2 -j16 --build-dir=../osx-build --stagedir=../osx-build/stage --prefix=$PREFIXDIR toolset=clang cxxflags="-std=c++11 -stdlib=libc++ -arch ${ARCH}" linkflags="-stdlib=libc++" link=static threading=multi stage
    ./b2 -j16 --build-dir=../osx-build --stagedir=../osx-build/stage --prefix=$PREFIXDIR toolset=clang cxxflags="-std=c++11 -stdlib=libc++ -arch ${ARCH}" linkflags="-stdlib=libc++" link=static threading=multi install
    doneSection
}

#===============================================================================
# Execution starts here
#===============================================================================

mkdir -p $OSXBUILDDIR

cleanEverythingReadyToStart
updateBoost

echo "BOOST_LIBS:        $BOOST_LIBS"
echo "BOOST_SRC:         $BOOST_SRC"
echo "OSXBUILDDIR:       $OSXBUILDDIR"
echo "PREFIXDIR:         $PREFIXDIR"
echo "XCODE_ROOT:        $XCODE_ROOT"
echo "COMPILER:          $COMPILER"
echo

bootstrapBoost
buildBoostForOSX

echo "Completed successfully"

#===============================================================================


#===============================================================================

cd $SRCDIR

mkdir -p ../mac/lib
for i in osx-build/stage/lib/libboost_*.a; do
    lib=`basename $i`
    echo "lib: $lib"
    rm -f ../mac/lib/$lib
    cp $i ../mac/lib
done

rm -rf ../mac/include/boost
cp -a osx/prefix/include/boost ../include
