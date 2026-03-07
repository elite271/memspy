#include "PointerScanWindow.h"
#include "imgui.h"

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
	ImGui::Text("Test");
}
