#include "ProcessHandle.h"


static uintptr_t GetModuleBaseAddress(DWORD procID)
{
	MODULEENTRY32 moduleEntry = { 0 };
	moduleEntry.dwSize = sizeof(MODULEENTRY32);

	HANDLE processSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procID);

	if (processSnapshot == INVALID_HANDLE_VALUE)
		return reinterpret_cast<uintptr_t>(nullptr);

	if (Module32First(processSnapshot, &moduleEntry))
	{
		return reinterpret_cast<uintptr_t>(moduleEntry.modBaseAddr);
	}

	CloseHandle(processSnapshot);

	return reinterpret_cast<uintptr_t>(nullptr);
}

ProcessHandle::ProcessHandle(const Process& process)
{
	processID = process.processID;

	processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);

	if (processHandle == nullptr)
	{
		printf("Process could not open with all access opening with different access %d", GetLastError());

		processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
	}

	if (processHandle == nullptr)
	{
		printf("Couldnt get handle to process %d", GetLastError());
	}

	ModuleBase = GetModuleBaseAddress(processID);

	if (!ModuleBase)
	{
		printf("Could not get module base address. %d", GetLastError());
	}
}

ProcessHandle::~ProcessHandle()
{
	printf("Handle closed!\n");
	CloseHandle(processHandle);
}

ProcessHandle::ProcessHandle(ProcessHandle&& other) noexcept
	: processHandle(other.processHandle), processID(other.processID)
{
	other.processHandle = nullptr;
	other.processID = 0;
}

ProcessHandle& ProcessHandle::operator=(ProcessHandle&& other) noexcept
{
	if (this != &other) 
	{
		if (processHandle != nullptr) 
		{
			CloseHandle(processHandle);
		}

		processHandle = other.processHandle;
		processID = other.processID;
		other.processHandle = nullptr;
		other.processID = 0;
	}

	return *this;
}

HANDLE ProcessHandle::GetHandle() const 
{
	return processHandle; 
}

DWORD ProcessHandle::GetProcessID() const 
{ 
	return processID; 
}

bool ProcessHandle::IsValid() const 
{ 
	return processHandle != nullptr;
}

uintptr_t ProcessHandle::GetModBase() const
{
	return ModuleBase;
}
