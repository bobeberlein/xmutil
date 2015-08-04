REM @echo off

SET "ICU_PATH=C:\icu"
SET "BOOST_PATH=C:\Program Files\boost\boost_1_58_0"

del /F /Q ".\include\*.*"
del /F /Q ".\lib64\*.*"

FOR /D %%G IN ("%ICU_PATH%\include\*") DO (
    mklink /D  ".\include\%%~nxG" "%%G"
)

mkdir ".\include\boost\"
FOR /D %%G IN ("%BOOST_PATH%\boost\*") DO (
    mklink /D  ".\include\boost\%%~nxG" "%%G"
)

FOR %%G IN ("%ICU_PATH%\lib64\*") DO (
    mklink /H  ".\lib64\%%~nxG" "%%G"
)

FOR %%G IN ("%BOOST_PATH%\lib64\*") DO (
    mklink /H  ".\lib64\%%~nxG" "%%G"
)

