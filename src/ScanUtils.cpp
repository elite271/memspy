#include "ScanUtils.h"
#include <algorithm>
#include <cctype>

ScanUtils::ScanUtils()
{
}

ScanUtils::~ScanUtils()
{
}

std::vector<ScanResult> ScanUtils::ScanForString(ProcessHandle& processHandle, const MemoryRegions& regions,
	const std::string& targetString, bool caseSensitive)
{
	std::vector<ScanResult> results;

	if (targetString.empty())
		return results;

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

		// Convert buffer to string for searching
		std::string bufferStr(reinterpret_cast<char*>(buffer.data()), bytesRead);

		// Search for the string
		size_t pos = 0;
		while (pos < bufferStr.length())
		{
			size_t found;

			if (caseSensitive)
			{
				found = bufferStr.find(targetString, pos);
			}
			else
			{
				// Case-insensitive search
				auto it = std::search(
					bufferStr.begin() + pos, bufferStr.end(),
					targetString.begin(), targetString.end(),
					[](char ch1, char ch2) {
						return std::tolower(static_cast<unsigned char>(ch1)) ==
							std::tolower(static_cast<unsigned char>(ch2));
					}
				);

				if (it == bufferStr.end())
					found = std::string::npos;
				else
					found = std::distance(bufferStr.begin(), it);
			}

			if (found == std::string::npos)
				break;

			uintptr_t address = reinterpret_cast<uintptr_t>(region.baseAddress) + found;
			ScanResult result;
			result.address = address;
			result.regionIndex = regionIdx;
			result.storedValue.resize(targetString.length());
			std::memcpy(result.storedValue.data(), &buffer[found], targetString.length());
			results.push_back(result);

			pos = found + 1; // Continue searching after this match
		}
	}

	return results;
}
