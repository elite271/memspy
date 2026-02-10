#include "ProcessList.h"
#include "Platform.h"

#if PLATFORM_WINDOWS
#include <TlHelp32.h>
#endif


ProcessList::ProcessList()
{
}

ProcessList::~ProcessList()
{
}

bool ProcessList::scanForProcesses()
{

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hSnapshot == INVALID_HANDLE_VALUE)
        return false;

    PROCESSENTRY32 pe;

    pe.dwSize = sizeof(PROCESSENTRY32);


    if (Process32First(hSnapshot, &pe))
    {
        do
        {
            Process info;

            info.processID = pe.th32ProcessID;

            info.currentProcessName = pe.szExeFile;

            if (info.processID != 0 && info.processID != 4)
            {
                processes.push_back(info);
            }

        } while (Process32Next(hSnapshot, &pe));
    }

    CloseHandle(hSnapshot);

    return true;
}

bool ProcessList::init()
{
    return scanForProcesses();
}

bool ProcessList::Refresh()
{
    Clear();

	return scanForProcesses();
}

void ProcessList::Clear()
{
	this->processes.clear();
}

Process ProcessList::at(size_t i)
{
    return processes.at(i);
}

size_t ProcessList::size()
{
    return processes.size();
}


