#!/bin/sh
if uname -a | grep -q 'MINGW'; then
	#windows  
	export GYP_MSVS_VERSION='2017'
	export QTDIR="C:/Qt/x64/Qt5.9.3/5.9.3/msvc2017_64"
else

if  uname -a | grep -q 'Linux'; then
	export QTDIR="$HOME/Qt5.10.0/5.10.0/gcc_64"
else
	#mac
	export QTDIR="$HOME/Qt5.9.3/5.9.3/clang_64"

    rm -f ./third_party/include/QtGui
	ln -s -f $QTDIR/lib/QtGui.framework/Headers ./third_party/include/QtGui
    rm -f ./third_party/include/QtCore
	ln -s -f $QTDIR/lib/QtCore.framework/Headers ./third_party/include/QtCore
    rm -f ./third_party/include/QtWidgets
	ln -s -f $QTDIR/lib/QtWidgets.framework/Headers ./third_party/include/QtWidgets
fi
fi
