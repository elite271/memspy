@echo off
REM Quick build script for Windows

echo ========================================
echo MemoryScanner Build Script (Windows)
echo ========================================
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

echo Configuring CMake...
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
echo Tip: If SDL3 isn't found, specify path with:
echo   build.bat "C:\path\to\SDL3-3.2.26"
echo Or set environment variable:
echo   set SDL3_PATH=C:\path\to\SDL3-3.2.26
pause
