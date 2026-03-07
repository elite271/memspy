#include "ProcessHandle.h"

#ifdef WIN32
#include <psapi.h>
#pragma comment(lib, "psapi.lib")
#endif


static uintptr_t GetModuleBaseAddress(DWORD procID)
{
    MODULEENTRY32 moduleEntry = { 0 };
    moduleEntry.dwSize = sizeof(MODULEENTRY32);

    HANDLE processSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procID);

	if (processSnapshot == INVALID_HANDLE_VALUE)
	{
		return reinterpret_cast<uintptr_t>(nullptr);
	}

    uintptr_t baseAddr = 0;

	if (Module32First(processSnapshot, &moduleEntry))
	{
		baseAddr = reinterpret_cast<uintptr_t>(moduleEntry.modBaseAddr);
	}
    
    CloseHandle(processSnapshot);

    return baseAddr;
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

std::vector<ModuleInfo> ProcessHandle::GetModuleList() const
{
	std::vector<ModuleInfo> modules;

#ifdef WIN32
	HMODULE hMods[1024];
	DWORD cbNeeded;

	if (!EnumProcessModules(processHandle, hMods, sizeof(hMods), &cbNeeded))
	{
		return modules;
	}

	for (int i = 0; i < cbNeeded / sizeof(HMODULE); i++)
	{
		MODULEINFO modInfo;
		if (!GetModuleInformation(processHandle, hMods[i], &modInfo, sizeof(modInfo)))
			continue;

		char name[256];
		GetModuleFileNameExA(processHandle, hMods[i], name, sizeof(name));

		modules.push_back({
			reinterpret_cast<uintptr_t>(modInfo.lpBaseOfDll),
			modInfo.SizeOfImage,
			std::string(name)
			});
	}
#endif

	return modules;
}
