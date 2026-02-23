cmake_minimum_required(VERSION 3.15)
project(MemoryScanner VERSION 1.0.0 LANGUAGES C CXX)

# Set C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set C11 standard for Zydis
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Platform detection
if(WIN32)
    set(PLATFORM_WINDOWS TRUE)
elseif(UNIX AND NOT APPLE)
    set(PLATFORM_LINUX TRUE)
endif()

# SDL3 Configuration
# You can set SDL3_PATH via command line: cmake -DSDL3_PATH=/path/to/sdl3
# Or set SDL3_DIR to point to the CMake config: cmake -DSDL3_DIR=/path/to/sdl3/cmake/SDL3
set(SDL3_PATH "" CACHE PATH "Path to SDL3 installation directory (e.g., E:/sdl3/SDL3-3.2.26)")

if(SDL3_PATH)
    # User specified SDL3_PATH, look for SDL3 there
    message(STATUS "Using custom SDL3 path: ${SDL3_PATH}")

    # Try common subdirectories where SDL3Config.cmake might be located
    set(SDL3_SEARCH_PATHS
        "${SDL3_PATH}"
        "${SDL3_PATH}/cmake"
        "${SDL3_PATH}/lib/cmake/SDL3"
        "${SDL3_PATH}/SDL3/cmake"
    )

    find_package(SDL3 REQUIRED CONFIG PATHS ${SDL3_SEARCH_PATHS} NO_DEFAULT_PATH)
else()
    # Try to find SDL3 in system paths
    find_package(SDL3 QUIET CONFIG)

    if(NOT SDL3_FOUND)
        message(STATUS "SDL3 not found in system paths. Trying common installation locations...")

        # Common Windows SDL3 locations
        if(WIN32)
            file(GLOB SDL3_CANDIDATE_PATHS
                "C:/SDL3*/lib/cmake/SDL3"
                "E:/sdl3/*/lib/cmake/SDL3"
                "$ENV{USERPROFILE}/SDL3*/lib/cmake/SDL3"
            )
        endif()

        find_package(SDL3 QUIET CONFIG PATHS ${SDL3_CANDIDATE_PATHS})
    endif()

    if(NOT SDL3_FOUND)
        message(FATAL_ERROR
            "SDL3 not found! Please specify SDL3 location:\n"
            "  Option 1: Set SDL3_PATH to SDL3 installation directory:\n"
            "    cmake -DSDL3_PATH=C:/path/to/SDL3-3.2.26 -B build\n"
            "  Option 2: Set SDL3_DIR to the CMake config directory:\n"
            "    cmake -DSDL3_DIR=C:/path/to/SDL3-3.2.26/cmake -B build\n"
            "  Option 3: Install SDL3 system-wide (Linux: sudo apt install libsdl3-dev)\n"
        )
    endif()
endif()

# ImGui sources
set(IMGUI_SOURCES
    include/imgui/imgui.cpp
    include/imgui/imgui_demo.cpp
    include/imgui/imgui_draw.cpp
    include/imgui/imgui_stdlib.cpp
    include/imgui/imgui_tables.cpp
    include/imgui/imgui_widgets.cpp
    include/imgui/imgui_impl_sdl3.cpp
    include/imgui/imgui_impl_sdlrenderer3.cpp
    include/imgui/imgui_hex.cpp
)

# Zydis sources
set(ZYDIS_SOURCES
    include/zydis/Zydis.c
)

# Application sources
set(APP_SOURCES
    src/main.cpp
    src/DisassemblyView.cpp
    src/MemoryHexWindow.cpp
    src/MemoryRegions.cpp
    src/MemoryRegionsView.cpp
    src/MemoryScannerApp.cpp
    src/Process.cpp
    src/ProcessHandle.cpp
    src/ProcessList.cpp
    src/ProcessSelectorWindow.cpp
    src/ScannerWindow.cpp
    src/ScanUtils.cpp
)

# Application headers
set(APP_HEADERS
    src/DisassemblyView.h
    src/MemoryHexWindow.h
    src/MemoryRegions.h
    src/MemoryRegionsView.h
    src/MemoryScannerApp.h
    src/Process.h
    src/ProcessHandle.h
    src/ProcessList.h
    src/ProcessSelectorWindow.h
    src/ScannerWindow.h
    src/ScanUtils.h
    include/imgui/imgui_stdlib.h
    "include/zydis/Zydis.h"
)

# Create executable
add_executable(MemoryScanner
    ${IMGUI_SOURCES}
    ${ZYDIS_SOURCES}
    ${APP_SOURCES}
    ${APP_HEADERS}
)

# Include directories
target_include_directories(MemoryScanner PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/include/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/include/zydis
)

# Link SDL3
target_link_libraries(MemoryScanner PRIVATE SDL3::SDL3)

# Platform-specific settings
if(PLATFORM_WINDOWS)
    # Windows-specific settings
    target_compile_definitions(MemoryScanner PRIVATE
        WIN32
        _CONSOLE
        ZYDIS_STATIC_BUILD
    )

    # Use MultiByte character set (not Unicode) to match existing code
    # The code uses std::string with Windows API, not std::wstring

    # Console subsystem
    if(MSVC)
        set_target_properties(MemoryScanner PROPERTIES
            LINK_FLAGS "/SUBSYSTEM:CONSOLE"
        )
    endif()

    # Automatically copy SDL3.dll to output directory after build
    add_custom_command(TARGET MemoryScanner POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SDL3::SDL3>
            $<TARGET_FILE_DIR:MemoryScanner>
        COMMENT "Copying SDL3.dll to output directory"
    )

elseif(PLATFORM_LINUX)
    # Linux-specific settings
    target_compile_definitions(MemoryScanner PRIVATE
        ZYDIS_STATIC_BUILD
    )

    # Link required libraries for Linux
    target_link_libraries(MemoryScanner PRIVATE
        pthread
        dl
    )

    # Note: Process management code needs to be implemented for Linux
    # Current implementation uses Windows APIs (OpenProcess, CreateToolhelp32Snapshot, etc.)
    # Linux port will need to use /proc filesystem and ptrace
    message(WARNING "Linux process management APIs not yet implemented. You'll need to add Linux-specific code for ProcessHandle and ProcessList.")
endif()

# Compiler warnings
if(MSVC)
    target_compile_options(MemoryScanner PRIVATE /W3)
else()
    target_compile_options(MemoryScanner PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Output directories
set_target_properties(MemoryScanner PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)

# Installation rules (optional)
install(TARGETS MemoryScanner
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# Print configuration summary
message(STATUS "")
message(STATUS "Configuration Summary:")
message(STATUS "  Project: ${PROJECT_NAME} v${PROJECT_VERSION}")
message(STATUS "  Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "  C++ Standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Install Prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "")
