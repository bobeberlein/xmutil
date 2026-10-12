@echo off
rem Builds the Windows XMUtil release program and runs the tests.
rem
rem   Output: out\msvs\Release\XMUtil.exe, with the ICU DLLs it needs beside it.
rem
rem   Any missing third-party material is fetched first: the ICU binaries
rem   (third_party\win\lib, third_party\include\unicode) and the tinyxml2
rem   sources (third_party\include). Downloads go to out\third_party.
rem
rem   Needs Visual Studio 2022 or later with the Desktop development with C++
rem   workload. Its bundled CMake is used when cmake is not on PATH. Windows 10+
rem   provides the curl and tar used for the downloads.
rem
rem Usage: build.bat [--skip-tests]

setlocal EnableExtensions EnableDelayedExpansion
cd /d "%~dp0"
set "ROOT=%CD%"

set "ICU_VERSION=78.3"
set "ICU_MAJOR=78"
set "TINYXML2_VERSION=8.0.0"
set "WORK=%ROOT%\out\third_party"
set "BUILD=%ROOT%\out\msvs"

set "RUN_TESTS=yes"
if /i "%~1"=="--skip-tests" set "RUN_TESTS=no"

rem ---------------------------------------------------------------------------
rem Tools. ProgramFiles(x86) holds a ")" so it is only ever expanded delayed.
rem ---------------------------------------------------------------------------
set "PF86=%ProgramFiles(x86)%"
set "VSWHERE=!PF86!\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "!VSWHERE!" (
    echo error: Visual Studio was not found. Install Visual Studio 2022 or later
    echo        with the Desktop development with C++ workload.
    exit /b 1
)
set "VSROOT="
for /f "usebackq delims=" %%i in (`""!VSWHERE!" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath"`) do set "VSROOT=%%i"
if not defined VSROOT (
    echo error: Visual Studio has no C++ toolset. Add the Desktop development
    echo        with C++ workload in the Visual Studio Installer.
    exit /b 1
)

set "CMAKE=cmake"
set "CTEST=ctest"
where cmake >nul 2>&1
if errorlevel 1 (
    set "CMAKE=!VSROOT!\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
    set "CTEST=!VSROOT!\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\ctest.exe"
    if not exist "!CMAKE!" (
        echo error: cmake was not found on PATH or in Visual Studio. Install the
        echo        C++ CMake tools for Windows component, or CMake from cmake.org.
        exit /b 1
    )
)
for %%t in (curl tar) do (
    where %%t >nul 2>&1
    if errorlevel 1 (
        echo error: %%t is required; it ships with Windows 10 and later.
        exit /b 1
    )
)

if not exist "%WORK%" mkdir "%WORK%"
if not exist "%ROOT%\third_party\include" mkdir "%ROOT%\third_party\include"

rem ---------------------------------------------------------------------------
rem ICU: the official prebuilt Win64 binaries.
rem ---------------------------------------------------------------------------
set "ICU_LIB=%ROOT%\third_party\win\lib"
set "ICU_NEEDED=no"
for %%l in (icudt icuin icuio icutu icuuc) do if not exist "%ICU_LIB%\%%l.lib" set "ICU_NEEDED=yes"
if not exist "%ICU_LIB%\dlls\icuuc*.dll" set "ICU_NEEDED=yes"
if not exist "%ROOT%\third_party\include\unicode\uvernum.h" set "ICU_NEEDED=yes"
if "%ICU_NEEDED%"=="yes" call :get_icu || exit /b 1

rem ---------------------------------------------------------------------------
rem tinyxml2: compiled from source into each target (no prebuilt lib).
rem ---------------------------------------------------------------------------
for %%f in (tinyxml2.cpp tinyxml2.h) do (
    if not exist "%ROOT%\third_party\include\%%f" (
        echo.
        echo ==^> Downloading %%f %TINYXML2_VERSION%
        curl -fsSL -o "%ROOT%\third_party\include\%%f" "https://raw.githubusercontent.com/leethomason/tinyxml2/%TINYXML2_VERSION%/%%f"
        if errorlevel 1 (
            echo error: downloading %%f failed
            exit /b 1
        )
    )
)

rem ---------------------------------------------------------------------------
rem Configure, build, test. No -G: CMake picks the newest Visual Studio.
rem ---------------------------------------------------------------------------
echo.
echo ==^> Configuring
"!CMAKE!" -S "%ROOT%" -B "%BUILD%" -A x64
if errorlevel 1 exit /b 1

echo.
echo ==^> Building Release
"!CMAKE!" --build "%BUILD%" --config Release --parallel
if errorlevel 1 exit /b 1

if "%RUN_TESTS%"=="yes" (
    echo.
    echo ==^> Testing Release
    "!CTEST!" --test-dir "%BUILD%" -C Release --output-on-failure
    if errorlevel 1 exit /b 1
)

echo.
echo ==^> Done
echo   %BUILD%\Release\XMUtil.exe
exit /b 0

rem ---------------------------------------------------------------------------
:get_icu
set "ICU_ZIP=icu4c-%ICU_VERSION%-Win64-MSVC2022.zip"
set "ICU_DIR=%WORK%\icu-%ICU_VERSION%-win64"
if not exist "%ICU_DIR%\lib64\icuuc.lib" (
    echo.
    echo ==^> Downloading ICU %ICU_VERSION%
    curl -fsSL -o "%WORK%\%ICU_ZIP%" "https://github.com/unicode-org/icu/releases/download/release-%ICU_VERSION%/%ICU_ZIP%"
    if errorlevel 1 (
        echo error: downloading %ICU_ZIP% failed
        exit /b 1
    )
    if exist "%ICU_DIR%" rmdir /s /q "%ICU_DIR%"
    mkdir "%ICU_DIR%"
    tar -xf "%WORK%\%ICU_ZIP%" -C "%ICU_DIR%"
    if errorlevel 1 (
        echo error: extracting %ICU_ZIP% failed
        exit /b 1
    )
)
if not exist "%ICU_LIB%\dlls" mkdir "%ICU_LIB%\dlls"
for %%l in (icudt icuin icuio icutu icuuc) do (
    copy /y "%ICU_DIR%\lib64\%%l.lib" "%ICU_LIB%\" >nul || exit /b 1
    rem Only the five ICU DLLs: CMake copies every DLL in this folder.
    copy /y "%ICU_DIR%\bin64\%%l%ICU_MAJOR%.dll" "%ICU_LIB%\dlls\" >nul || exit /b 1
)
if not exist "%ROOT%\third_party\include\unicode\uvernum.h" (
    xcopy /e /i /q /y "%ICU_DIR%\include\unicode" "%ROOT%\third_party\include\unicode" >nul || exit /b 1
)
exit /b 0
