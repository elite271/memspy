#pragma once

#include <SDL3/SDL.h>
#include <stdio.h>
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
#include "imgui_hex.h"
#include "MemoryRegions.h"
#include <vector>

class MemoryHexWindow
{
public:
	MemoryHexWindow();
	~MemoryHexWindow();

	bool Init();

	void OnWindowResize(float width, float height);

	void Render(MemoryRegions* regions, std::optional<ProcessHandle>& procHandle);

	// Set memory data to display
	void SetMemoryData(void* data, int size);

	// Highlight a range of bytes
	void HighlightRange(int from, int to, ImColor color);

	// Clear all highlights
	void ClearHighlights();

	// 
	bool ReadBytesFromRegion(const MemoryRegion& region, std::optional<ProcessHandle>& procHandle);

private:
	ImGuiHexEditorState editorState;
	std::vector<unsigned char> testData;  // Test data for demonstration
	std::vector<std::string> regionLabels;
	int currentRegionIdx = -1;
};
