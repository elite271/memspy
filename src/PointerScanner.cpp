#include "PointerScanner.h"

PointerScanner::PointerScanner()
{
}

PointerScanner::~PointerScanner()
{
}

uintptr_t PointerScanner::ReadPointer(HANDLE hProcess, uintptr_t address)
{
	return uintptr_t();
}

bool PointerScanner::IsValidPointer(uintptr_t value)
{
#ifdef _WIN64
	return value >= 0x10000 && value <= 0x00007FFFFFFFFFFF && (value % 4 == 0);
#else
	return value >= 0x10000 && value <= 0x7FFFFFFF && (value % 4 == 0);
#endif
}

std::vector<std::pair<uintptr_t, uintptr_t>> PointerScanner::FindPointersToAddress(
	HANDLE hProcess, const MemoryRegions& regions, uintptr_t target, uintptr_t maxOffset)
{
	return std::vector<std::pair<uintptr_t, uintptr_t>>();
}

uintptr_t PointerScanner::FindOwningModuleBase(HANDLE hProcess, uintptr_t address)
{
	return uintptr_t();
}

void PointerScanner::Scan(ProcessHandle& handle, const MemoryRegions& regions, uintptr_t target)
{
}
