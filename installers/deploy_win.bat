
rmdir /s/q ventrans
mkdir ventrans
copy ..\Release\XMUtil.exe ventrans
copy ..\Release\*.dll ventrans

C:\Qt\x64\Qt5.11.0\5.11.0\msvc2017_64\bin\windeployqt "ventrans/XMUtil.exe"
