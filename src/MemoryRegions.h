#pragma once

#include "Platform.h"
#include <vector>
#include <optional>
#include "ProcessHandle.h"
#include "Process.h"

enum class MemoryRegionType : uint8_t {
	Stack,
	Heap
};

struct MemoryRegion {
	const void* baseAddress = nullptr;
	size_t regionSize;
	DWORD regionProtection;
};


class MemoryRegions
{
public:
	MemoryRegions(std::optional<ProcessHandle>& procHandle);
	~MemoryRegions();
	MemoryRegions(const MemoryRegions&) = delete;
	MemoryRegions& operator=(const MemoryRegions&) = delete;
	MemoryRegions(MemoryRegions&&) noexcept;
	MemoryRegions& operator=(const MemoryRegions&&) noexcept;

	void SetSizes(size_t minSize = 1000, size_t maxSize = 1000);

	bool Refresh(MemoryRegionType type, std::optional<ProcessHandle> &procHandle);

	bool isValid();

	const std::vector<MemoryRegion>& GetRegions() const { return memoryRegions; }

private:
	std::vector<MemoryRegion> memoryRegions;

	size_t minSize = 0;
	size_t maxSize = SIZE_MAX;

	bool ValidRegions = false;
};


