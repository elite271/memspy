#pragma once

#include "ProcessSelectorWindow.h"
#include "ScannerWindow.h"
#include "MemoryHexWindow.h"
#include "MemoryRegionsView.h"
#include "DisassemblyView.h"
#include "MemoryRegions.h"
#include <optional>

class MemoryScannerApp
{
public:
	MemoryScannerApp();
	~MemoryScannerApp();

	bool Init();

	void OnWindowResize(float width, float height);

	void Update();

	void Render();

private:
	mem::processselector::ProcessSelectorWindow selectorWindow;
	ScannerWindow scannerWindow;
	MemoryHexWindow hexWindow;
	MemoryRegionsView regionsView;
	DisassemblyView disassemblyView;

	// Shared state
	std::optional<ProcessHandle> attachedProcess;
	std::optional<MemoryRegions> memoryRegions;
	std::string currentProcessName = "no Process Attached";

	// UI state
	bool showSelector = true;
	bool showScanner = false;
};

