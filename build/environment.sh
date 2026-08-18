#!/bin/sh
if uname -a | grep -q 'MINGW'; then
	#windows
	# VS 2017 and later stop populating the HKLM\...\VisualStudio\SxS keys gyp
	# probes, so hand it the install root from vswhere: with
	# GYP_MSVS_OVERRIDE_PATH set, gyp skips registry detection entirely.
	if [ -z "${GYP_MSVS_VERSION:-}" ]; then
		vswhere="/c/Program Files (x86)/Microsoft Visual Studio/Installer/vswhere.exe"
		if [ -x "$vswhere" ]; then
			vs_root=$("$vswhere" -latest -products '*' \
				-requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 \
				-property installationPath | tr -d '\r')
			vs_year=$("$vswhere" -latest -products '*' \
				-requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 \
				-property catalog_productLineVersion | tr -d '\r')
		fi
	fi
	export GYP_MSVS_VERSION="${GYP_MSVS_VERSION:-${vs_year:-2017}}"
	if [ -n "${vs_root:-}" ] && [ -z "${GYP_MSVS_OVERRIDE_PATH:-}" ]; then
		export GYP_MSVS_OVERRIDE_PATH="$vs_root"
	fi
	export QTDIR="${QTDIR:-C:/Qt/x64/Qt5.11.0/5.11.0/msvc2017_64}"
elif  uname -a | grep -q 'Linux'; then
	export QTDIR="$HOME/Qt5.11.0/5.11.0/gcc_64"
else
	#mac
	if [ $mac_arch = x86_64 ]; then 
    	export QTDIR="$HOME/Qt5.15.4/5.15.4/clang_64"
    else
    	export QTDIR="$HOME/Qt5.15.4/5.15.4/clang_arm64"
    fi

	rm -f ./third_party/include/QtGui
	mkdir -p third_party/include
	ln -s -f $QTDIR/lib/QtGui.framework/Headers ./third_party/include/QtGui
	rm -f ./third_party/include/QtCore
	ln -s -f $QTDIR/lib/QtCore.framework/Headers ./third_party/include/QtCore
	rm -f ./third_party/include/QtWidgets
	ln -s -f $QTDIR/lib/QtWidgets.framework/Headers ./third_party/include/QtWidgets
fi
