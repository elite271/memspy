#include "MemoryRegions.h"

MemoryRegions::MemoryRegions(std::optional<ProcessHandle>& procHandle)
{
	if (!Refresh(MemoryRegionType::Heap, procHandle))
	{
		printf("Could not get memory regions\n");
		ValidRegions = false;
	}
	else
	{
		ValidRegions = true;
	}
}

MemoryRegions::~MemoryRegions()
{
}

MemoryRegions::MemoryRegions(MemoryRegions&& other) noexcept
	: memoryRegions(other.memoryRegions), minSize(other.minSize),
	maxSize(other.maxSize), ValidRegions(other.ValidRegions)
{
	other.memoryRegions.clear();
	other.minSize = 0;
	other.maxSize = 0;
	other.ValidRegions = false;
}

MemoryRegions& MemoryRegions::operator=(const MemoryRegions&& other) noexcept
{
	if (this != &other)
	{
		if (!memoryRegions.empty())
		{
			memoryRegions.clear();
		}

		minSize = other.minSize;
		maxSize = other.maxSize;
		/*ValidRegions = other.ValidRegions;*/
		ValidRegions = false;
	}

	return *this;
}

void MemoryRegions::SetSizes(size_t minSize, size_t maxSize)
{
	this->minSize = minSize;
	this->maxSize = maxSize;
}

bool MemoryRegions::Refresh(MemoryRegionType type, std::optional<ProcessHandle>& procHandle)
{
	if (!procHandle.has_value())
	{
		printf("Process handle doesnt have a value");

		return false;
	}

	if (!procHandle->IsValid())
	{
		printf("Invalid process handle in memory regions");

		return false;
	}

	// Clear existing regions before scanning
	memoryRegions.clear();

	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);

	uintptr_t offset = reinterpret_cast<uintptr_t>(sysInfo.lpMinimumApplicationAddress);
	const uintptr_t maxAddress = reinterpret_cast<uintptr_t>(sysInfo.lpMaximumApplicationAddress);

	printf("DEBUG: Scanning memory from 0x%llX to 0x%llX\n",
	       static_cast<unsigned long long>(offset),
	       static_cast<unsigned long long>(maxAddress));

	MEMORY_BASIC_INFORMATION mbi;

	size_t totalRegions = 0;
	size_t filteredRegions = 0;

	// Continue scanning even if VirtualQueryEx fails for some regions
	while (offset < maxAddress)
	{
		SIZE_T result = VirtualQueryEx(procHandle->GetHandle(), reinterpret_cast<LPCVOID>(offset), &mbi, sizeof(mbi));

		// If VirtualQueryEx fails, skip to next page
		if (result == 0)
		{
			offset += sysInfo.dwPageSize;
			continue;
		}

		totalRegions++;

		// Debug: Print first few regions
		if (totalRegions <= 10)
		{
			printf("  Region %zu: Base=0x%p, Size=%zu bytes (0x%llX), State=%d, Protect=0x%X\n",
			       totalRegions, mbi.BaseAddress, mbi.RegionSize,
			       static_cast<unsigned long long>(mbi.RegionSize), mbi.State, mbi.Protect);
		}

		// More relaxed filtering - include any committed, readable memory
		bool isCommitted = (mbi.State == MEM_COMMIT);
		bool isReadable = (mbi.Protect & (PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READWRITE |
		                                   PAGE_EXECUTE_WRITECOPY | PAGE_READONLY | PAGE_EXECUTE_READ));
		bool notGuarded = !(mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS));
		bool sizeOk = (mbi.RegionSize >= minSize && mbi.RegionSize <= maxSize);

		if (isCommitted && isReadable && notGuarded && sizeOk)
		{
			memoryRegions.push_back({ mbi.BaseAddress, mbi.RegionSize, mbi.Protect });
			filteredRegions++;
		}

		uintptr_t nextOffset = reinterpret_cast<uintptr_t>(mbi.BaseAddress) + mbi.RegionSize;

		// Debug: Check for potential overflow or huge jump
		if (totalRegions <= 10)
		{
			printf("    Next offset: 0x%llX (jumped %lld bytes)\n",
			       static_cast<unsigned long long>(nextOffset),
			       static_cast<long long>(nextOffset - offset));
		}

		offset = nextOffset;
	}

	printf("Memory scan: %zu total regions, %zu passed filters, %zu added to scan list\n",
	       totalRegions, filteredRegions, memoryRegions.size());
	return true;
}

bool MemoryRegions::isValid()
{
	return ValidRegions;
}




