REM @echo off
REM http://sourceforge.net/projects/winflexbison/
REM http://site.icu-project.org/download

REM - must be run as administrator

SET "ICU_PATH=D:\tools\icu"
SET "BOOST_PATH=D:\tools\boost"
SET "BISON_PATH=D:\tools\bison\bin"
SET "TINY_XML_PATH=D:\tools\tinyxml2"

 IF NOT EXIST ".\include" GOTO NOINCLUDE
   rmdir  /s /q ".\include"
   :NOINCLUDE

 IF NOT EXIST ".\bison" GOTO NOBISON
   rmdir  /s /q ".\bison"
   :NOBISON

 IF NOT EXIST ".\lib32" GOTO NOLIB32
   rmdir  /s /q ".\lib32"
   :NOLIB32

mkdir ".\include"
mkdir ".\include\boost\"
mkdir ".\lib32"

FOR /D %%G IN ("%ICU_PATH%\include\*") DO (
    mklink /D  ".\include\%%~nxG" "%%G"
)

FOR %%G IN ("%ICU_PATH%\bin\*.dll") DO (
    mklink  ".\lib32\%%~nxG" "%%G"
)

FOR %%G IN ("%ICU_PATH%\lib\*") DO (
    mklink  ".\lib32\%%~nxG" "%%G"
)

FOR /D %%G IN ("%BOOST_PATH%\boost\*") DO (
    mklink /D  ".\include\boost\%%~nxG" "%%G"
)

FOR %%G IN ("%BOOST_PATH%\boost\*") DO (
    mklink  ".\include\boost\%%~nxG" "%%G"
)

FOR %%G IN ("%BOOST_PATH%\lib32-msvc-12.0\*") DO (
    mklink  ".\lib32\%%~nxG" "%%G"
)

FOR %%G IN ("%TINY_XML_PATH%\*.h") DO (
    mklink  ".\include\%%~nxG" "%%G"
)

FOR %%G IN ("%TINY_XML_PATH%\*.c*") DO (
    mklink  ".\include\%%~nxG" "%%G"
)


FOR %%G IN ("%TINY_XML_PATH%\lib32\*") DO (
    mklink  ".\lib32\%%~nxG" "%%G"
)

mklink /D ".\bison" "%BISON_PATH%"

