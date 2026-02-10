#pragma once

#include "Platform.h"
#include "Process.h"

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


private:
	HANDLE processHandle = nullptr;
	DWORD processID = 0;
};


