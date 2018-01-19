#!/bin/bash

gypfile="${GYPFILE:-XMUtil.gyp}"
rmout="${rmout:-yes}"

source ./build/environment.bash

quiet=no
msvs=no
xcode=no

usage() {
    cat <<EOF
usage: $0 [--use-xcode] [--use-msvs] [-q]

This bootstraps the gyp build system.
EOF
}

for arg in "$@"
do
    case $arg in
    --use-xcode)
    xcode=yes
    ;;
    --use-msvs)
    msvs=yes
    ;;
    -q)
    quiet=yes
    ;;
    --help)
    usage
    exit
    ;;
    esac
done

if [ $xcode = 'yes' ]; then
    generator=xcode
elif [ $msvs = 'yes' ]; then
    generator=msvs
else
    generator=ninja
fi

GYP_GENERATORS=$generator 

"./build/bin/gyp" -Dcwd=`pwd` $GYPDEFS $gypfile --toplevel-dir=`pwd` --depth=0
result=$?

if [ $quiet = 'no' ]; then
    echo "configured with:"
    echo "  xcode generation:        $xcode"
    echo "  msvs generation:         $msvs"
    echo "  CC:                      ${CC:-<default>}"
    echo "  CXX:                     ${CXX:-<default>}"
fi

exit $result
