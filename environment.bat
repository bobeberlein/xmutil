if defined ProgramFiles(x86) (
    call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Professional\VC\Auxiliary\Build\vcvars64.bat"
) else (
    call "C:\Program Files\Microsoft Visual Studio\2017\Professional\VC\Auxiliary\Build\vcvars32.bat"
)
if %1.==. (
    call "C:\MinGW\msys\1.0\msys.bat"
) else (
    call "C:\MinGW\msys\1.0\msys.bat" %1 %2 %3 %4 %5 %6 %7 %8
)
