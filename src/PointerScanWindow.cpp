#ifdef WIN32
#include <Windows.h>
#include <process.h>
#endif

#include "PointerScanWindow.h"
#include "imgui.h"
#include "imgui_stdlib.h"
#include <thread>

PointerScanWindow::PointerScanWindow()
{
}

PointerScanWindow::~PointerScanWindow()
{
}

bool PointerScanWindow::Init()
{
	return true;
}

void PointerScanWindow::Render(MemoryRegions* regions, std::optional<ProcessHandle>& procHandle)
{
    if (!procHandle.has_value() || !regions || !regions->isValid())
    {
        return;
    }

    ImGui::Text("Target Address:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(200.0f);
    ImGui::InputText("##TargetAddr", &targetAddressInput);

    ImGui::SameLine();
    ImGui::Text("Max Offset:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(80.0f);
    ImGui::InputText("##MaxOffset", &maxOffsetInput);

    ImGui::SameLine();
    ImGui::Text("Max Depth:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(100.0f);
    ImGui::InputInt("##MaxDepth", &maxDepth);

    ImGui::SameLine();
    if (ImGui::Button("Scan"))
    {
        try
        {
            uintptr_t target = std::stoull(targetAddressInput, nullptr, 16);
            uintptr_t maxOffset = std::stoull(maxOffsetInput, nullptr, 16);
            if (procHandle.has_value())
            {
                isScanning = true;
                std::thread([this, &procHandle, regions, target, maxOffset]() {
                    scanner.Scan(procHandle.value(), *regions, target, maxOffset, maxDepth);
                    isScanning = false;
                    }).detach();
            }
        }
        catch (const std::exception& e)
        {
            printf("Invalid input: %s\n", e.what());
        }
    }

    ImGui::SameLine();
    ImGui::Text("Results: %zu", scanner.GetResults().size());

    ImGui::Separator();

    // Results table
    if (!scanner.GetResults().empty())
    {
        if (ImGui::BeginTable("PtrResults", 2,
            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
            ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable))
        {
            ImGui::TableSetupColumn("Module", ImGuiTableColumnFlags_WidthFixed, 200.0f);
            ImGui::TableSetupColumn("Chain", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableHeadersRow();

            for (const auto& chain : scanner.GetResults())
            {
                if (chain.moduleName.find(procHandle->GetProcessName()) == std::string::npos)
                {
                    continue;
                }

                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%s+0x%llX", chain.moduleName.c_str(), chain.moduleOffset);

                ImGui::TableSetColumnIndex(1);
                // Build offset chain string
                std::string chainStr;
                for (size_t i = 0; i < chain.offsets.size(); i++)
                {
                    if (i > 0) chainStr += " -> ";
                    char buf[32];
                    snprintf(buf, sizeof(buf), "0x%llX", chain.offsets[i]);
                    chainStr += buf;
                }
                ImGui::Text("%s", chainStr.c_str());
            }

            ImGui::EndTable();
        }
    }
    else if (isScanning)
    {
        ImGui::Text("Scanning...");
    }
    else
    {
        ImGui::Text("Enter a target address and click Scan.");
    }
}
