#include "MemoryScannerApp.h"

MemoryScannerApp::MemoryScannerApp()
{
}

MemoryScannerApp::~MemoryScannerApp()
{
}

bool MemoryScannerApp::Init()
{
    return selectorWindow.Init() &&
        scannerWindow.Init() &&
        hexWindow.Init() &&
        regionsView.Init() &&
        disassemblyView.Init();
}

void MemoryScannerApp::OnWindowResize(float width, float height)
{
    ImGui::SetNextWindowSize(ImVec2(width, height));
}

void MemoryScannerApp::Update()
{
}

void MemoryScannerApp::Render()
{
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(io.DisplaySize);

    // const std::string titleText = "Memory Scanner - "  + currentProcessName;


    // "Memory Scanner"
    ImGui::Begin("Memory Scanner", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus);

    if (ImGui::BeginTabBar("MainTabs")) 
    {
        if (ImGui::BeginTabItem("Process Selector")) 
        {
            // selectorWindow.Render();
            selectorWindow.RenderTest(attachedProcess);

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Scanner"))
        {
            if (attachedProcess.has_value())
            {
                // Create MemoryRegions if we have a process but no regions yet
                if (!memoryRegions.has_value())
                {
                    memoryRegions.emplace(attachedProcess);
                }

                scannerWindow.Render(attachedProcess.value(), &memoryRegions.value());
            }
            else
            {
                ImGui::Text("No process attached. Select a process first.");
                // Clear memory regions if process was detached
                if (memoryRegions.has_value())
                {
                    memoryRegions.reset();
                    disassemblyView.Clear();
                }
            }

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Regions"))
        {
            if (attachedProcess.has_value())
            {
                // Create MemoryRegions if we have a process but no regions yet
                if (!memoryRegions.has_value())
                {
                    memoryRegions.emplace(attachedProcess);
                }

                regionsView.Render(&memoryRegions.value());
            }
            else
            {
                ImGui::Text("No process attached. Select a process first.");
            }

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Hex View"))
        {
            if (attachedProcess.has_value())
            {
                if (!memoryRegions.has_value())
                {
                    memoryRegions.emplace(attachedProcess);
                }

                hexWindow.Render(&memoryRegions.value(), attachedProcess);
            }
            else
            {
                ImGui::Text("No process attached. Select a process first.");
            }

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Disassembly"))
        {
            if (attachedProcess.has_value())
            {
                // Create MemoryRegions if we have a process but no regions yet
                if (!memoryRegions.has_value())
                {
                    memoryRegions.emplace(attachedProcess);
                }

                disassemblyView.Render(&memoryRegions.value(), attachedProcess);
            }
            else
            {
                ImGui::Text("No process attached. Select a process first.");
                // Clear disassembly view if process was detached
                disassemblyView.Clear();
            }

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}

//Then ProcessSelectorWindow.Render() and ScannerWindow.Render() just render their content without creating their own
// windows - they're rendering inside the tab content area.

// This gives you a clean Cheat Engine - style interface with tabs for different tools, all sharing the same attached
// process.Much cleaner than managing multiple separate windows!

