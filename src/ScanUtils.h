#pragma once

#include "Platform.h"
#include <optional>
#include <string>
#include <vector>
#include "ProcessHandle.h"
#include "MemoryRegions.h"

enum ScanType
{
	FirstScan,
	ExactValue,
	ChangedValue,
	UnchangedValue,
	IncreasedValue,
	DecreasedValue
};

struct ScanResult
{
	uintptr_t address;
	size_t regionIndex;
	std::vector<uint8_t> storedValue;  // Store bytes for comparison
};

class ScanUtils
{
public:
	ScanUtils();
	~ScanUtils();

	// Template function for scanning numeric types
	template <typename T>
	static std::vector<ScanResult> ScanForValue(ProcessHandle& processHandle, const MemoryRegions& regions, T targetValue)
	{
		std::vector<ScanResult> results;
		const auto& regionList = regions.GetRegions();

		for (size_t regionIdx = 0; regionIdx < regionList.size(); regionIdx++)
		{
			const auto& region = regionList[regionIdx];

			// Allocate buffer for this region
			std::vector<uint8_t> buffer(region.regionSize);

			// Read memory from the region
			SIZE_T bytesRead = 0;
			if (!ReadProcessMemory(processHandle.GetHandle(),
				region.baseAddress,
				buffer.data(),
				region.regionSize,
				&bytesRead))
			{
				continue; // Skip regions we can't read
			}

			// Scan the buffer for the target value
			const size_t scanLimit = bytesRead - sizeof(T) + 1;
			for (size_t i = 0; i < scanLimit; i++)
			{
				// Compare bytes at this position
				T* valuePtr = reinterpret_cast<T*>(&buffer[i]);
				if (*valuePtr == targetValue)
				{
					uintptr_t address = reinterpret_cast<uintptr_t>(region.baseAddress) + i;
					ScanResult result;
					result.address = address;
					result.regionIndex = regionIdx;
					result.storedValue.resize(sizeof(T));
					std::memcpy(result.storedValue.data(), valuePtr, sizeof(T));
					results.push_back(result);
				}
			}
		}

		return results;
	}

	// Template function for next scan operations
	template <typename T>
	static std::vector<ScanResult> NextScan(
		ProcessHandle& processHandle,
		const std::vector<ScanResult>& previousResults,
		ScanType scanType,
		std::optional<T> targetValue = std::nullopt)
	{
		std::vector<ScanResult> results;

		for (const auto& prevResult : previousResults)
		{
			// Read current value
			T currentValue;
			SIZE_T bytesRead = 0;
			if (!ReadProcessMemory(processHandle.GetHandle(),
				reinterpret_cast<LPCVOID>(prevResult.address),
				&currentValue, sizeof(T), &bytesRead) || bytesRead != sizeof(T))
			{
				continue; // Skip inaccessible addresses
			}

			bool matches = false;
			T storedValue;
			std::memcpy(&storedValue, prevResult.storedValue.data(), sizeof(T));

			switch (scanType)
			{
			case ExactValue:
				matches = (currentValue == targetValue.value());
				break;
			case ChangedValue:
				matches = (currentValue != storedValue);
				break;
			case UnchangedValue:
				matches = (currentValue == storedValue);
				break;
			case IncreasedValue:
				matches = (currentValue > storedValue);
				break;
			case DecreasedValue:
				matches = (currentValue < storedValue);
				break;
			default:
				break;
			}

			if (matches)
			{
				ScanResult result = prevResult;
				// Update stored value to current
				std::memcpy(result.storedValue.data(), &currentValue, sizeof(T));
				results.push_back(result);
			}
		}

		return results;
	}

	// String scanning (separate implementation)
	static std::vector<ScanResult> ScanForString(ProcessHandle& processHandle, const MemoryRegions& regions,
		const std::string& targetString, bool caseSensitive = true);

private:

};