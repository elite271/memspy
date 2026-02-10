#include "ProcessHandle.h"

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