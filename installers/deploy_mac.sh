#!/bin/sh

export QTDIR="$HOME/Qt5.11.0/5.11.0/clang_64"

mkdir -p "$PWD/out/Release"
rm -rf "$/out/Release/XMUtil.app"
cp -rf "$PWD/../DerivedData/XMUtil/Build/Products/Release/XMUtil.app" "$PWD/out/Release/XMUtil.app"

$QTDIR/bin/macdeployqt "$PWD/out/Release/XMUtil.app"
