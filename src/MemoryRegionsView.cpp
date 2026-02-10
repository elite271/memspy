#include "MemoryRegionsView.h"
#include "Platform.h"
#include <format>

MemoryRegionsView::MemoryRegionsView()
{
}

MemoryRegionsView::~MemoryRegionsView()
{
}

bool MemoryRegionsView::Init()
{
	return true;
}

void MemoryRegionsView::Render(MemoryRegions* regions)
{
	if (!regions || !regions->isValid())
	{
		ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "No memory regions loaded");
		return;
	}

	const auto& regionList = regions->GetRegions();

	ImGui::Text("Total regions: %zu", regionList.size());
	ImGui::Separator();

	// Create a table to display regions
	if (ImGui::BeginTable("RegionsTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable))
	{
		// Setup columns
		ImGui::TableSetupColumn("Base Address", ImGuiTableColumnFlags_WidthFixed, 150.0f);
		ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthFixed, 120.0f);
		ImGui::TableSetupColumn("Size (Bytes)", ImGuiTableColumnFlags_WidthFixed, 120.0f);
		ImGui::TableSetupColumn("Protection", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableSetupScrollFreeze(0, 1); // Freeze header row
		ImGui::TableHeadersRow();

		// Display each region
		for (const auto& region : regionList)
		{
			ImGui::TableNextRow();

			// Base Address
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("0x%p", region.baseAddress);

			// Size (formatted)
			ImGui::TableSetColumnIndex(1);
			if (region.regionSize >= 1024 * 1024)
				ImGui::Text("%.2f MB", region.regionSize / (1024.0 * 1024.0));
			else if (region.regionSize >= 1024)
				ImGui::Text("%.2f KB", region.regionSize / 1024.0);
			else
				ImGui::Text("%zu B", region.regionSize);

			// Size (bytes)
			ImGui::TableSetColumnIndex(2);
			ImGui::Text("%zu", region.regionSize);

			// Protection
			ImGui::TableSetColumnIndex(3);
			ImGui::Text("%s", GetProtectionString(region.regionProtection).c_str());
		}

		ImGui::EndTable();
	}
}

std::string MemoryRegionsView::GetProtectionString(DWORD protection)
{
	std::string result;

	if (protection & PAGE_EXECUTE_READWRITE)
		result += "RWX ";
	else if (protection & PAGE_EXECUTE_READ)
		result += "RX ";
	else if (protection & PAGE_EXECUTE_WRITECOPY)
		result += "WCX ";
	else if (protection & PAGE_EXECUTE)
		result += "X ";
	else if (protection & PAGE_READWRITE)
		result += "RW ";
	else if (protection & PAGE_READONLY)
		result += "R ";
	else if (protection & PAGE_WRITECOPY)
		result += "WC ";
	else if (protection & PAGE_NOACCESS)
		result += "--- ";

	if (protection & PAGE_GUARD)
		result += "[GUARD] ";
	if (protection & PAGE_NOCACHE)
		result += "[NOCACHE] ";
	if (protection & PAGE_WRITECOMBINE)
		result += "[WRITECOMBINE] ";

	return result.empty() ? "UNKNOWN" : result;
}