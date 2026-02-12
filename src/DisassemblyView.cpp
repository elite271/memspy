#include "DisassemblyView.h"

DisassemblyView::DisassemblyView()
{
}

DisassemblyView::~DisassemblyView()
{
}

bool DisassemblyView::Init()
{
	// Initialize Zydis decoder for x64
	ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_STACK_WIDTH_64);

	// Initialize formatter
	ZydisFormatterInit(&formatter, ZYDIS_FORMATTER_STYLE_INTEL);

	initialized = true;
	return true;
}

void DisassemblyView::Render(MemoryRegions* regions, std::optional<ProcessHandle>& procHandle)
{
	if (!initialized || !regions || !regions->isValid())
	{
		ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "No memory regions loaded");
		return;
	}

	// Enable/Disable toggle
	ImGui::Checkbox("Enable Disassembly##DisasmEnable", &enabled);
	ImGui::SameLine();
	ImGui::TextDisabled("(CPU usage can be high when enabled)");

	if (!enabled)
	{
		ImGui::Text("Disassembly view is disabled");
		return;
	}

	const auto& regionList = regions->GetRegions();

	// Regenerate region labels if count changed
	if ((int)regionList.size() != lastRegionCount)
	{
		regionLabels.clear();
		executableRegionIndices.clear();
		for (size_t i = 0; i < regionList.size(); ++i)
		{
			// Only include executable regions
			if (IsExecutableRegion(regionList[i].regionProtection))
			{
				char buffer[256];
				snprintf(buffer, sizeof(buffer), "0x%p (%zu bytes)", regionList[i].baseAddress, regionList[i].regionSize);
				regionLabels.push_back(std::string(buffer));
				executableRegionIndices.push_back((int)i);
			}
		}
		lastRegionCount = (int)regionList.size();
		selectedRegionIndex = -1;
		lastSelectedRegionIndex = -1;

		// Debug: show if no regions found
		if (regionLabels.empty())
		{
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "No executable regions found (found %zu total regions)", regionList.size());
			return;
		}
	}

	// Region selector dropdown
	ImGui::Text("Select Region:");
	ImGui::SameLine();

	std::vector<const char*> labelPtrs;
	for (const auto& label : regionLabels)
	{
		labelPtrs.push_back(label.c_str());
	}

	if (ImGui::Combo("##RegionSelector", &selectedRegionIndex, labelPtrs.data(), labelPtrs.size()))
	{
		// Disassemble the newly selected region
		if (selectedRegionIndex >= 0 && selectedRegionIndex < (int)executableRegionIndices.size())
		{
			int actualRegionIndex = executableRegionIndices[selectedRegionIndex];
			DisassembleRegion(regionList[actualRegionIndex], procHandle);
		}
	}

	// Only re-disassemble if selection changed
	if (selectedRegionIndex != lastSelectedRegionIndex && selectedRegionIndex >= 0 && selectedRegionIndex < (int)executableRegionIndices.size())
	{
		int actualRegionIndex = executableRegionIndices[selectedRegionIndex];
		DisassembleRegion(regionList[actualRegionIndex], procHandle);
		lastSelectedRegionIndex = selectedRegionIndex;
	}

	ImGui::Separator();

	// Display disassembly if we have instructions
	if (instructions.empty())
	{
		if (selectedRegionIndex >= 0)
		{
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Failed to disassemble region or region is empty");
		}
		else
		{
			ImGui::Text("Select a region to disassemble");
		}
		return;
	}

	size_t displayCount = instructions.size() > MAX_INSTRUCTIONS_TO_DISPLAY ? MAX_INSTRUCTIONS_TO_DISPLAY : instructions.size();
	ImGui::Text("Instructions: %zu", instructions.size());
	if (instructions.size() > MAX_INSTRUCTIONS_TO_DISPLAY)
	{
		ImGui::SameLine();
		ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "(showing first %zu of %zu)", displayCount, instructions.size());
	}
	ImGui::Separator();

	// Create table for disassembly display
	if (ImGui::BeginTable("DisassemblyTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable, ImVec2(0, 0)))
	{
		ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_WidthFixed, 120.0f);
		ImGui::TableSetupColumn("Bytes", ImGuiTableColumnFlags_WidthFixed, 180.0f);
		ImGui::TableSetupColumn("Mnemonic", ImGuiTableColumnFlags_WidthFixed, 80.0f);
		ImGui::TableSetupColumn("Operands", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableSetupScrollFreeze(0, 1);
		ImGui::TableHeadersRow();

		// Use clipper for virtual scrolling
		ImGuiListClipper clipper;
		clipper.Begin(displayCount);
		while (clipper.Step())
		{
			for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
			{
				const auto& instr = instructions[row];

				ImGui::TableNextRow();

				// Address
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("0x%llx", instr.address);

				// Bytes (hex)
				ImGui::TableSetColumnIndex(1);

				char bytesStr[512] = { 0 };
				for (size_t i = 0; i < instr.bytes.size() && i < 16; ++i)  // Limit to 16 bytes display
				{
					snprintf(bytesStr + strlen(bytesStr), sizeof(bytesStr) - strlen(bytesStr), "%02x ", instr.bytes[i]);
				}
				ImGui::Text("%s", bytesStr);

				// Mnemonic
				ImGui::TableSetColumnIndex(2);

				const auto& cat_color = GetCategoryColor(instr.category);

				switch (cat_color)
				{
				case CALL:
					ImGui::TextColored(ImVec4{0.4f, 0.8f, 1.0f, 1.0f}, "%s", instr.mnemonic.c_str());
					break;
				case JCC:
					ImGui::TextColored(ImVec4{1.0f, 1.0f, 0.0f, 1.0f}, "%s", instr.mnemonic.c_str());
					break;
				case JMP:
					ImGui::TextColored(ImVec4{1.0f, 0.4f, 1.0f, 1.0f}, "%s", instr.mnemonic.c_str());
					break;
				case RET:
					ImGui::TextColored(ImVec4{1.0f, 0.3f, 0.3f, 1.0f}, "%s", instr.mnemonic.c_str());
					break;
				default:
					ImGui::Text("%s", instr.mnemonic.c_str());
					break;
				}

				// Operands
				ImGui::TableSetColumnIndex(3);
				ImGui::TextColored(ImVec4{0.0f, 1.0f, 0.0f, 1.0f}, "%s", instr.operands.c_str());
			}
		}

		ImGui::EndTable();
	}
}

void DisassemblyView::DisassembleRegion(const MemoryRegion& region, std::optional<ProcessHandle>& procHandle)
{
	instructions.clear();

	if (!procHandle.has_value() || !region.baseAddress)
	{
		return;
	}

	// Read memory from region
	size_t bufferSize = region.regionSize > 0x10000 ? 0x10000 : region.regionSize; // Limit to 64KB for performance
	std::vector<uint8_t> buffer(bufferSize);
	SIZE_T bytesRead = 0;

	if (!ReadProcessMemory(procHandle->GetHandle(), region.baseAddress, buffer.data(), buffer.size(), &bytesRead))
	{
		return;
	}

	// Disassemble
	ZydisDecodedInstruction instruction;
	ZydisDecodedOperand operands[ZYDIS_MAX_OPERAND_COUNT_VISIBLE];
	size_t offset = 0;
	uintptr_t runtimeAddress = reinterpret_cast<uintptr_t>(region.baseAddress);

	while (offset < bytesRead && instructions.size() < MAX_INSTRUCTIONS_TO_DISPLAY)
	{
		ZyanStatus status = ZydisDecoderDecodeInstruction(&decoder, &decoderContext,
			buffer.data() + offset, bytesRead - offset, &instruction);

		if (!ZYAN_SUCCESS(status) || instruction.length == 0)
			break;

		DisassembledInstruction instr;
		instr.address = runtimeAddress + offset;
		instr.category = instruction.meta.category;

		// Store raw bytes
		for (uint8_t i = 0; i < instruction.length; ++i)
		{
			instr.bytes.push_back(buffer[offset + i]);
		}

		// Decode operands
		ZydisDecoderDecodeOperands(&decoder, &decoderContext, &instruction, operands, instruction.operand_count);

		// Format instruction
		char buffer_instr[256];
		ZydisFormatterFormatInstruction(&formatter, &instruction, operands, instruction.operand_count,
			buffer_instr, sizeof(buffer_instr), runtimeAddress + offset, nullptr);
		std::string fullMnemonic(buffer_instr);

		// Split mnemonic and operands
		size_t spacePos = fullMnemonic.find(' ');
		if (spacePos != std::string::npos)
		{
			instr.mnemonic = fullMnemonic.substr(0, spacePos);
			instr.operands = fullMnemonic.substr(spacePos + 1);
		}
		else
		{
			instr.mnemonic = fullMnemonic;
			instr.operands = "";
		}

		instructions.push_back(instr);
		offset += instruction.length;
	}
}

CategoryColor DisassemblyView::GetCategoryColor(ZydisInstructionCategory category) const
{
	switch (category)
	{
	case ZYDIS_CATEGORY_CALL:
		return CALL;
	case ZYDIS_CATEGORY_COND_BR:
		return JCC;
	case ZYDIS_CATEGORY_UNCOND_BR:
		return JMP;
	case ZYDIS_CATEGORY_RET:
		return RET;
	default:
		return NORMAL;
	}
}

bool DisassemblyView::IsExecutableRegion(DWORD protection) const
{
	// Check if region has execute permission
	return (protection & (PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY)) != 0;
}
