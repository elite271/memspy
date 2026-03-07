#pragma once

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

#include <cstdint>
#include <vector>
#include <iostream>
#include <unordered_set>

#include "MemoryRegions.h"

struct PointerChain
{
	std::string moduleName;
	uintptr_t moduleOffset;
	std::vector<uintptr_t> offsets;
};

struct PointerCandidate
{
	uintptr_t address;
	uintptr_t offset;
};

class PointerScanner
{
public:
	PointerScanner();
	~PointerScanner();

	static bool ReadPointer(HANDLE hProcess, uintptr_t address, uintptr_t& outValue);

	static bool IsValidPointer(uintptr_t value);

	static std::vector<PointerCandidate> FindPointersToAddress(
		HANDLE hProcess,
		const MemoryRegions& regions,
		uintptr_t target,
		uintptr_t maxOffset);

	static uintptr_t FindOwningModuleBase(HANDLE hProcess, uintptr_t address);

	void Scan(ProcessHandle& handle, const MemoryRegions& regions, uintptr_t target, uintptr_t maxOffset, int maxDepth);

	const std::vector<PointerChain>& GetResults() const { return results; }

private:

	struct Node
	{
		uintptr_t address;
		int parent;
		uintptr_t offset;
	};

	struct WorkItem
	{
		uintptr_t target;
		int depth;
		int nodeIdx;
	};

	std::vector<PointerChain> results;
};

