#pragma once
#include <MemoryRegions.h>

class PointerScanWindow
{
public:
	PointerScanWindow();
	~PointerScanWindow();

	bool Init();
	void Render(MemoryRegions* regions, std::optional<ProcessHandle>& procHandle);

private:

};

