#pragma once

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

#include <cstdint>
#include <vector>
#include <iostream>

#include "MemoryRegions.h"

struct PointerChain
{
	std::string moduleName;
	uintptr_t moduleOffset;
	std::vector<uintptr_t> offsets;
};


class PointerScanner
{
public:
	PointerScanner();
	~PointerScanner();

	static uintptr_t ReadPointer(HANDLE hProcess, uintptr_t address);

	static bool IsValidPointer(uintptr_t value);

	static std::vector<std::pair<uintptr_t, uintptr_t>> FindPointersToAddress(
		HANDLE hProcess,
		const MemoryRegions& regions,
		uintptr_t target,
		uintptr_t maxOffset);

	static uintptr_t FindOwningModuleBase(HANDLE hProcess, uintptr_t address);



	void Scan(ProcessHandle& handle, const MemoryRegions& regions, uintptr_t target);

private:
	std::vector<PointerChain> results;
};

