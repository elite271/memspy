#pragma once

#include "Platform.h"
#include "MemSpyProcess.h"

struct ModuleInfo
{
	uintptr_t base;
	uintptr_t size;
	std::string name;
};

class ProcessHandle
{
public:
	ProcessHandle(const Process& process);
	~ProcessHandle();
	ProcessHandle(const ProcessHandle&) = delete;
	ProcessHandle& operator=(const ProcessHandle&) = delete;
	ProcessHandle(ProcessHandle&&) noexcept;
	ProcessHandle& operator=(ProcessHandle&&) noexcept;

	HANDLE GetHandle() const; 
	DWORD GetProcessID() const;
	bool IsValid() const;
	uintptr_t GetModBase() const;
	std::vector<ModuleInfo> GetModuleList() const;

private:
	HANDLE processHandle = nullptr;
	DWORD processID = 0;
	uintptr_t ModuleBase;
};


