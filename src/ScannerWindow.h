#pragma once

#include <SDL3/SDL.h>
#include <stdio.h>
#include "imgui.h"
#include "imgui_stdlib.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
#include <vector>
#include <string>
#include <optional>
#include <iostream>
#include "MemoryRegions.h"
#include "ProcessHandle.h"
#include "ScanUtils.h"

enum ScanSize {
	Byte,
	TwoBytes,
	FourBytes,
	EightBytes,
	Float,
	Double,
	String,
	WString
};

enum StringType {
	Ascii,
	UTF8,
	UTF16,
	UTF32 // idk what one windows uses so to be safe put all of them
};

class ScannerWindow
{
public:
	ScannerWindow();
	~ScannerWindow();

	bool Init();

	void OnWindowResize(float width, float height);

	void OnScanButtonPressed(ProcessHandle& processHandle, MemoryRegions& regions);

	void Render(ProcessHandle& processHandle, MemoryRegions* regions);

	void ResetScanState();

private:
	std::string inputText;

	ScanSize currentScanSize = Byte;

	StringType stringEncoding = Ascii;

	std::vector<ScanResult> scanResults;
	bool isScanning = false;

	ScanType currentScanType = FirstScan;
	bool hasScannedOnce = false;

	// For value editing
	int selectedResultIndex = -1;
	bool showEditPopup = false;
	char editBuffer[64] = {};
};

