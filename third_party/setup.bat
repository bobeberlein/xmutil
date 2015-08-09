REM @echo off
REM http://sourceforge.net/projects/winflexbison/
REM http://site.icu-project.org/download

SET "ICU_PATH=C:\tools\icu"
SET "BOOST_PATH=C:\Program Files\boost\boost_1_58_0"
SET "BISON_PATH=C:\tools\bison\bin"

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
    mklink /H  ".\lib32\%%~nxG" "%%G"
)

FOR %%G IN ("%ICU_PATH%\lib\*") DO (
    mklink /H  ".\lib32\%%~nxG" "%%G"
)

FOR /D %%G IN ("%BOOST_PATH%\boost\*") DO (
    mklink /D  ".\include\boost\%%~nxG" "%%G"
)

FOR %%G IN ("%BOOST_PATH%\boost\*") DO (
    mklink /H  ".\include\boost\%%~nxG" "%%G"
)

FOR %%G IN ("%BOOST_PATH%\lib32\*") DO (
    mklink /H  ".\lib32\%%~nxG" "%%G"
)

mklink /D ".\bison" "%BISON_PATH%"

