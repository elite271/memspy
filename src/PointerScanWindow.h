#pragma once
#include <MemoryRegions.h>
#include "PointerScanner.h"

class PointerScanWindow
{
public:
	PointerScanWindow();
	~PointerScanWindow();

	bool Init();
	void Render(MemoryRegions* regions, std::optional<ProcessHandle>& procHandle);

private:
	PointerScanner scanner;
	std::string targetAddressInput;
	std::string maxOffsetInput = "500";
	int maxDepth = 1; // originally 4
	bool isScanning = false;
};

