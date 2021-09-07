#!/bin/sh
set -euo pipefail

gypfile="${GYPFILE:-XMUtil.gyp}"
rmout="${rmout:-yes}"

source ./build/environment.sh

quiet=no
msvs=no
xcode=no
ui=no
gyp_ui=0
arch_id=64
x86=yes


usage() {
    cat <<EOF
usage: $0 [--use-xcode] [--use-msvs] [-q] [--with-ui]

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
	--32bit)
		arch_id=32
		x86=yes
	;;
	--64bit)
		arch_id=64
		x86=no
    ;;
    --with-ui)
    ui=yes
    gyp_ui=1
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

if [ $ui = 'yes' ]; then
    . ./qt_generate.sh
fi

if [ $xcode = 'yes' ]; then
    generator=xcode
elif [ $msvs = 'yes' ]; then
    generator=msvs
else
    generator=ninja
fi
if [[ $arch_id = '32' &&  $x86 = 'yes' ]]; then
    export CC="${CC:-cc} -m32 -march=prescott"
    export CXX="${CXX:-cxx} -m32 -march=prescott"
fi


export GYP_GENERATORS=$generator 
export GYPDEFS="-Dqtdir=$QTDIR -Dwith_ui=$gyp_ui"

"./build/bin/gyp" -Darch_id=$arch_id  -Dcwd=`pwd` $GYPDEFS $gypfile --toplevel-dir=`pwd` --depth=0
result=$?


if [ $quiet = 'no' ]; then
    echo "configured with:"
    echo "  xcode generation:        $xcode"
    echo "  msvs generation:         $msvs"
    echo "  with ui:                 $ui"
    echo "  x86:                     $x86"
    echo "  CC:                      ${CC:-<default>}"
    echo "  CXX:                     ${CXX:-<default>}"
fi

exit $result
