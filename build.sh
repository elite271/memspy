#!/bin/bash
# Quick build script for Linux

echo "========================================"
echo "MemoryScanner Build Script (Linux)"
echo "========================================"
echo ""

# Check if build directory exists
if [ -d "build" ]; then
    echo "Build directory exists. Cleaning..."
    rm -rf build
fi

# Check for SDL3_PATH from environment or argument
SDL3_ARG=""
if [ ! -z "$SDL3_PATH" ]; then
    echo "Using SDL3_PATH from environment: $SDL3_PATH"
    SDL3_ARG="-DSDL3_PATH=$SDL3_PATH"
fi

if [ ! -z "$1" ]; then
    echo "Using SDL3_PATH from argument: $1"
    SDL3_ARG="-DSDL3_PATH=$1"
fi

echo "Configuring CMake..."
cmake -B build -DCMAKE_BUILD_TYPE=Release $SDL3_ARG

if [ $? -ne 0 ]; then
    echo ""
    echo "CMake configuration failed!"
    echo "Make sure CMake and SDL3 development packages are installed."
    echo "  Ubuntu/Debian: sudo apt-get install cmake libsdl3-dev"
    exit 1
fi

echo ""
echo "Building..."
cmake --build build -j$(nproc)

if [ $? -ne 0 ]; then
    echo ""
    echo "Build failed!"
    exit 1
fi

echo ""
echo "========================================"
echo "Build successful!"
echo "Executable: build/bin/MemoryScanner"
echo "========================================"
echo ""
echo "Run with: ./build/bin/MemoryScanner"
echo ""
echo "Tip: If SDL3 isn't found, specify path with:"
echo "  ./build.sh /path/to/SDL3"
echo "Or set environment variable:"
echo "  export SDL3_PATH=/path/to/SDL3"
