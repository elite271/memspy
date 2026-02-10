#pragma once

#include "Platform.h"
#include "imgui.h"
#include "MemoryRegions.h"
#include <string>

class MemoryRegionsView
{
public:
	MemoryRegionsView();
	~MemoryRegionsView();

	bool Init();

	void Render(MemoryRegions* regions);

private:
	std::string GetProtectionString(DWORD protection);
};



