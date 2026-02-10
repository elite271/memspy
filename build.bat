@echo off
REM Quick build script for Windows
REM Usage: build.bat [x64|x86] [SDL3_PATH]

echo ========================================
echo MemoryScanner Build Script (Windows)
echo ========================================
echo.

REM Default to x64
set ARCH=x64
if /i "%1"=="x86" (
    set ARCH=x86
    shift
)
if /i "%1"=="x64" (
    set ARCH=x64
    shift
)

echo Target Architecture: %ARCH%
echo.

REM Set up MSVC environment for the target architecture (Windows only)
if defined VSCMD_ARG_TGT_ARCH (
    if /i "%VSCMD_ARG_TGT_ARCH%"=="%ARCH%" (
        echo MSVC environment already configured for %ARCH%
        echo.
        goto skip_vcvars
    ) else (
        echo MSVC configured for %VSCMD_ARG_TGT_ARCH% but need %ARCH%, reconfiguring...
    )
)

echo Setting up MSVC %ARCH% environment...

REM Try to find and run vcvarsall.bat
if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" (
    call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" %ARCH% >nul 2>&1
    goto skip_vcvars
)

if exist "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvarsall.bat" (
    call "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvarsall.bat" %ARCH% >nul 2>&1
    goto skip_vcvars
)

if exist "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" (
    call "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" %ARCH% >nul 2>&1
    goto skip_vcvars
)

echo Warning: Could not find vcvarsall.bat - if build fails, run from "x64 Native Tools Command Prompt"

:skip_vcvars
echo.

REM Check if build directory exists
if exist build (
    echo Build directory exists. Cleaning...
    rmdir /s /q build
)

REM Check for SDL3_PATH environment variable or argument
set SDL3_ARG=
if not "%SDL3_PATH%"=="" (
    echo Using SDL3_PATH from environment: %SDL3_PATH%
    set SDL3_ARG=-DSDL3_PATH=%SDL3_PATH%
)

if not "%1"=="" (
    echo Using SDL3_PATH from argument: %1
    set SDL3_ARG=-DSDL3_PATH=%1
)

echo Configuring CMake for %ARCH%...
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release %SDL3_ARG%

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo CMake configuration failed!
    echo Make sure CMake and SDL3 are installed.
    pause
    exit /b 1
)

echo.
echo Building...
cmake --build build

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo Build failed!
    pause
    exit /b 1
)

echo.
echo ========================================
echo Build successful!
echo Executable: build\bin\MemoryScanner.exe
echo ========================================
echo.
echo Run with: .\build\bin\MemoryScanner.exe
echo.
echo Usage examples:
echo   build.bat                                     (builds x64 by default)
echo   build.bat x86                                 (builds 32-bit)
echo   build.bat x64 "C:\path\to\SDL3-3.2.26"       (x64 with custom SDL3 path)
echo.
echo Tip: If SDL3 isn't found, specify path with:
echo   build.bat x64 "C:\path\to\SDL3-3.2.26"
echo Or set environment variable:
echo   set SDL3_PATH=C:\path\to\SDL3-3.2.26
pause
