# Building MemoryScanner

This project uses CMake as its build system, supporting both Windows and Linux.

## Prerequisites

### Common Requirements
- CMake 3.15 or higher
- C++20 compatible compiler
- SDL3 library

### Windows
- Visual Studio 2019/2022 with C++ Desktop Development workload, or
- MinGW-w64, or
- Clang for Windows

### Linux
- GCC 10+ or Clang 12+
- Development packages: `build-essential`, `libsdl3-dev`

**Note:** The current codebase uses Windows-specific APIs for process management. Linux support requires implementing platform-specific code for:
- `ProcessHandle.cpp` - Replace `OpenProcess`, `CloseHandle` with Linux equivalents
- `ProcessList.cpp` - Replace `CreateToolhelp32Snapshot`, `Process32First/Next` with `/proc` filesystem parsing
- `MemoryRegions.cpp` - Implement memory region enumeration using `/proc/<pid>/maps`

## Building on Windows

### Architecture Selection (IMPORTANT!)

**MemoryScanner must be built for the same architecture as the target process:**
- **x64 (64-bit)** - Required for scanning modern applications (Chrome, Spotify, Discord, etc.)
- **x86 (32-bit)** - Only for scanning older 32-bit applications

**Default is x64.** A 32-bit build can only see a tiny portion of a 64-bit process's memory (0x10000-0x7FFEFFFF), which will result in finding only 2-5 memory regions instead of hundreds or thousands.

### Quick Build (Recommended)
```bash
# Build x64 (default - for modern 64-bit applications)
.\build.bat

# Build x64 with custom SDL3 path
.\build.bat x64 "E:\sdl3\SDL3-3.2.26"

# Build x86 (for older 32-bit applications)
.\build.bat x86 "E:\sdl3\SDL3-3.2.26"

# Or set environment variable
set SDL3_PATH=E:\sdl3\SDL3-3.2.26
.\build.bat x64
```

### Using Visual Studio
```bash
# Configure
cmake -B build -G "Visual Studio 17 2022" -A x64

# If SDL3 is in a custom location, add:
cmake -B build -G "Visual Studio 17 2022" -A x64 -DSDL3_PATH="E:\sdl3\SDL3-3.2.26"

# Build
cmake --build build --config Release

# Run
.\build\bin\Release\MemoryScanner.exe
```

### Using Ninja (faster)

**Important:** With Ninja, you must run from the correct Visual Studio command prompt for your target architecture:
- For x64: Open "x64 Native Tools Command Prompt for VS 2022"
- For x86: Open "x86 Native Tools Command Prompt for VS 2022"

```bash
# Configure
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release

# If SDL3 is in a custom location, add:
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DSDL3_PATH="E:\sdl3\SDL3-3.2.26"

# Build
cmake --build build

# Run
.\build\bin\MemoryScanner.exe
```

## Building on Linux

**Important:** You must first implement Linux-specific process management code before building on Linux.

### Quick Build
```bash
# Install dependencies (Ubuntu/Debian)
sudo apt-get install build-essential cmake libsdl3-dev

# Build
./build.sh

# If SDL3 is in a custom location
./build.sh /path/to/SDL3

# Or set environment variable
export SDL3_PATH=/path/to/SDL3
./build.sh
```

### Manual Build
```bash
# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release

# If SDL3 is in a custom location, add:
cmake -B build -DCMAKE_BUILD_TYPE=Release -DSDL3_PATH=/opt/SDL3

# Build
cmake --build build -j$(nproc)

# Run
./build/bin/MemoryScanner
```

## CMake Presets (Modern Alternative)

If you have CMake 3.21+, you can use presets for a cleaner workflow:

```bash
# List available presets
cmake --list-presets

# Configure with a preset
cmake --preset windows-release  # or linux-release

# Build with a preset
cmake --build --preset windows-release
```

### Custom SDL3 Path with Presets

You can use the provided example file:

```bash
# Copy the example file
copy CMakeUserPresets.json.example CMakeUserPresets.json  # Windows
# Or
cp CMakeUserPresets.json.example CMakeUserPresets.json    # Linux

# Edit CMakeUserPresets.json and update SDL3_PATH to your installation

# Then build with your custom preset
cmake --preset my-windows-release
cmake --build --preset my-windows-release
```

Or create your own `CMakeUserPresets.json` (this file is gitignored):

```json
{
  "version": 3,
  "configurePresets": [
    {
      "name": "my-windows",
      "inherits": "windows-release",
      "cacheVariables": {
        "SDL3_PATH": "E:/sdl3/SDL3-3.2.26"
      }
    }
  ]
}
```

Then build with: `cmake --preset my-windows && cmake --build --preset my-windows`

## Build Options

### Build Types
- `Debug` - Debug symbols, no optimization
- `Release` - Optimized, no debug symbols
- `RelWithDebInfo` - Optimized with debug symbols
- `MinSizeRel` - Optimized for size

Example:
```bash
cmake -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
```

### Custom SDL3 Location

There are three ways to specify a custom SDL3 location:

**Option 1: SDL3_PATH (Recommended - easiest)**
```bash
# Point to SDL3 installation directory
cmake -B build -DSDL3_PATH="E:\sdl3\SDL3-3.2.26"
```

**Option 2: SDL3_DIR (Advanced)**
```bash
# Point directly to CMake config directory
cmake -B build -DSDL3_DIR="E:\sdl3\SDL3-3.2.26\cmake"
# Or
cmake -B build -DSDL3_DIR="E:\sdl3\SDL3-3.2.26\lib\cmake\SDL3"
```

**Option 3: Environment Variable**
```bash
# Windows
set SDL3_PATH=E:\sdl3\SDL3-3.2.26
cmake -B build

# Linux/Mac
export SDL3_PATH=/opt/SDL3
cmake -B build
```

CMake will automatically search these locations within SDL3_PATH:
- `${SDL3_PATH}/`
- `${SDL3_PATH}/cmake/`
- `${SDL3_PATH}/lib/cmake/SDL3/`
- `${SDL3_PATH}/SDL3/cmake/`

## Dependencies Included

The following dependencies are included in the repository:
- **ImGui** - UI framework (`include/imgui/`)
- **Zydis** - Disassembler library (`include/zydis/`)

## Clean Build

```bash
# Remove build directory
rm -rf build

# Or on Windows
rmdir /s /q build
```

## IDE Integration

### Visual Studio
Open the generated `.sln` file in the build directory, or use "Open Folder" with CMake support.

### Visual Studio Code
Install the CMake Tools extension and open the project folder.

### CLion
Open the project folder directly - CLion has native CMake support.

## Troubleshooting

### Only Finding 2-5 Memory Regions in Target Process

**Symptom:** When scanning a 64-bit application like Spotify or Chrome, only 2-5 regions are found with addresses around 0x7FFE0000.

**Cause:** You built a 32-bit (x86) version of MemoryScanner, which can only access a small portion of a 64-bit process's address space (0x10000 to 0x7FFEFFFF).

**Solution:** Rebuild as 64-bit:
```bash
# Delete old build
rmdir /s /q build

# Rebuild for x64
.\build.bat x64 "E:\sdl3\SDL3-3.2.26"
```

**How to verify:** After fixing, you should see:
- Debug output showing address range up to `0x00007FFFFFFEFFFF` (not `0x7FFEFFFF`)
- Hundreds or thousands of memory regions found (not just 2-5)

### SDL3 Not Found

If you get an error like `SDL3 not found!`, try these solutions:

**Windows:**
1. Download SDL3 development libraries from [libsdl.org](https://www.libsdl.org/)
2. Extract to a directory (e.g., `E:\sdl3\SDL3-3.2.26`)
3. Build with: `cmake -B build -DSDL3_PATH="E:\sdl3\SDL3-3.2.26"`

Expected SDL3 directory structure:
```
SDL3-3.2.26/
├── include/
│   └── SDL3/
│       ├── SDL.h
│       └── ...
├── lib/
│   └── x64/
│       ├── SDL3.lib
│       └── SDL3.dll
└── cmake/  (or lib/cmake/SDL3/)
    └── SDL3Config.cmake
```

**Linux:**
```bash
# Ubuntu/Debian
sudo apt-get install libsdl3-dev

# If not available, build from source:
git clone https://github.com/libsdl-org/SDL
cd SDL
cmake -B build -DCMAKE_INSTALL_PREFIX=/usr/local
sudo cmake --build build --target install
```

### C++20 Errors
- Ensure your compiler supports C++20
- GCC 10+, Clang 12+, or MSVC 19.28+ required

### Linker Errors on Linux
If you encounter linker errors, ensure you have the necessary system libraries:
```bash
sudo apt-get install libpthread-stubs0-dev
```
