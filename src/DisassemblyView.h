#pragma once

#include <SDL3/SDL.h>
#include <stdio.h>
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
#include <vector>
#include <string>
#include <optional>
#include "MemoryRegions.h"
#include "ProcessHandle.h"
// #include "include/zydis/Zydis.h"
#include "Zydis.h"

struct DisassembledInstruction
{
	uintptr_t address;
	std::vector<uint8_t> bytes;
	std::string mnemonic;
	std::string operands;
	ZydisInstructionCategory category;
};

enum CategoryColor 
{
	CALL,
	JCC,
	JMP,
	RET,
	NORMAL
};

class DisassemblyView
{
public:
	DisassemblyView();
	~DisassemblyView();

	bool Init();

	void Render(MemoryRegions* regions, std::optional<ProcessHandle>& procHandle);

	void SetSelectedRegionIndex(int index) { selectedRegionIndex = index; }

	void Clear()
	{
		instructions.clear();
		regionLabels.clear();
		executableRegionIndices.clear();
		selectedRegionIndex = -1;
		lastSelectedRegionIndex = -1;
		lastRegionCount = -1;
		enabled = true;
	}

private:
	void DisassembleRegion(const MemoryRegion& region, std::optional<ProcessHandle>& procHandle);
	CategoryColor GetCategoryColor(ZydisInstructionCategory category) const;
	bool IsExecutableRegion(DWORD protection) const;

	std::vector<DisassembledInstruction> instructions;
	std::vector<std::string> regionLabels;
	std::vector<int> executableRegionIndices; // Maps dropdown index to actual region index
	int selectedRegionIndex = -1;
	int lastSelectedRegionIndex = -1;
	int lastRegionCount = -1;
	ZydisDecoder decoder;
	ZydisFormatter formatter;
	ZydisDecoderContext decoderContext;
	bool initialized = false;
	bool enabled = false; // start the disassembly view disabled to make sure the program doesnt go crazy
	static constexpr size_t MAX_INSTRUCTIONS_TO_DISPLAY = 2000;
};


