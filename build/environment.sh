#!/bin/sh
if uname -a | grep -q 'MINGW'; then
	#windows  
	export GYP_MSVS_VERSION='2017'
	export QTDIR="C:/Qt/x64/Qt5.11.0/5.11.0/msvc2017_64"
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
