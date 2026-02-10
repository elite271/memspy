#include "MemoryHexWindow.h"
#include "Platform.h"

MemoryHexWindow::MemoryHexWindow()
{
}

MemoryHexWindow::~MemoryHexWindow()
{
}

bool MemoryHexWindow::Init()
{
	// Setup editor state
	editorState.Bytes = testData.data();
	editorState.MaxBytes = testData.size();
	editorState.ShowAscii = true;
	editorState.ShowAddress = true;
	editorState.ReadOnly = false;
	editorState.Separators = 8;  // Add separator every 8 bytes
	// editorState.OptShowDataPreview = false;
	editorState.RenderZeroesDisabled = false;  // Show zeros normally
	editorState.LowercaseBytes = false;  // Uppercase hex

	return true;
}

void MemoryHexWindow::OnWindowResize(float width, float height)
{
	// No special handling needed - the hex editor adapts automatically
}

void MemoryHexWindow::Render(MemoryRegions* regions, std::optional<ProcessHandle>& procHandle)
{
	const auto& regionsList = regions->GetRegions();

	// Clear region labels each frame to prevent duplicates
	regionLabels.clear();

	char buffer[256];

	for (auto i = 0u; i < regionsList.size(); i++)
	{
		snprintf(buffer, sizeof(buffer), "0x%p (%zu bytes)", regionsList.at(i).baseAddress, regionsList.at(i).regionSize);
		regionLabels.push_back(buffer);
	}

	std::vector<const char*> labelPtrs;
	for (const auto& label : regionLabels)
	{
		labelPtrs.push_back(label.c_str());
	}

	if (ImGui::Combo("##HexRegions", &currentRegionIdx, labelPtrs.data(), labelPtrs.size()))
	{
		// Check if index is valid before accessing
		if (currentRegionIdx >= 0 && currentRegionIdx < static_cast<int>(regionsList.size()))
		{
			if (!ReadBytesFromRegion(regionsList.at(currentRegionIdx), procHandle))
			{
				printf("Read bytes failed");

				return;
			}
		}
	}
	// Render the hex editor - it will fill the available content region
	if (ImGui::BeginHexEditor("##HexEditor", &editorState, ImVec2(0, 0)))
	{
		// BeginHexEditor returns true if the editor is visible
		SetMemoryData(testData.data(), static_cast<int>(testData.size()));
	}
	ImGui::EndHexEditor();
}

void MemoryHexWindow::SetMemoryData(void* data, int size)
{
	editorState.Bytes = data;
	editorState.MaxBytes = size;
}

void MemoryHexWindow::HighlightRange(int from, int to, ImColor color)
{
	ImGuiHexEditorHighlightRange range;
	range.From = from;
	range.To = to;
	range.Color = color;
	range.BorderColor = ImColor(255, 255, 255, 255);
	range.Flags = ImGuiHexEditorHighlightFlags_Apply |
	              ImGuiHexEditorHighlightFlags_FullSized |
	              ImGuiHexEditorHighlightFlags_Border;

	editorState.HighlightRanges.push_back(range);
}

void MemoryHexWindow::ClearHighlights()
{
	editorState.HighlightRanges.clear();
}

bool MemoryHexWindow::ReadBytesFromRegion(const MemoryRegion& region, std::optional<ProcessHandle>& procHandle)
{
	testData.clear();

	if (!procHandle.has_value() || !region.baseAddress)
	{
		return false;
	}

	size_t bufferSize = region.regionSize > 0x10000 ? 0x10000 : region.regionSize;
	SIZE_T bytesRead = 0;

	testData.resize(bufferSize);

	if (!ReadProcessMemory(procHandle->GetHandle(), region.baseAddress, testData.data(), testData.size(), &bytesRead))
	{
		printf("Read proc failed in readbytes from region.\n");

		return false;
	}

	return true;
}
