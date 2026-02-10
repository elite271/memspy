#pragma once

// Platform detection and abstraction layer
// This header provides platform-specific types and includes

#if defined(_WIN32) || defined(_WIN64)
    #define PLATFORM_WINDOWS 1
    #define PLATFORM_LINUX 0

    // Windows includes
    #include <Windows.h>
    #include <TlHelp32.h>

    // Platform types (already defined by Windows.h)
    // HANDLE, DWORD, etc.

#elif defined(__linux__)
    #define PLATFORM_WINDOWS 0
    #define PLATFORM_LINUX 1

    // Linux includes
    #include <sys/types.h>
    #include <dirent.h>
    #include <unistd.h>
    #include <sys/ptrace.h>
    #include <sys/wait.h>

    // Platform type aliases for Linux
    // TODO: Implement Linux equivalents
    using HANDLE = int;  // Use file descriptor or process ID
    using DWORD = uint32_t;
    using BOOL = int;

    // Windows API equivalents that need to be implemented for Linux:
    // - OpenProcess() -> Linux: open /proc/<pid>/mem, use ptrace
    // - CloseHandle() -> Linux: close file descriptor
    // - CreateToolhelp32Snapshot() -> Linux: read /proc directory
    // - Process32First/Next() -> Linux: iterate /proc entries
    // - VirtualQueryEx() -> Linux: parse /proc/<pid>/maps
    // - ReadProcessMemory() -> Linux: pread on /proc/<pid>/mem
    // - WriteProcessMemory() -> Linux: pwrite on /proc/<pid>/mem

    #ifndef FALSE
    #define FALSE 0
    #endif

    #ifndef TRUE
    #define TRUE 1
    #endif

    #ifndef INVALID_HANDLE_VALUE
    #define INVALID_HANDLE_VALUE (-1)
    #endif

#else
    #error "Unsupported platform"
#endif
