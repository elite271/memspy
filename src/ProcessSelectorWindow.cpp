#include "ProcessSelectorWindow.h"

namespace mem::processselector
{
	ProcessSelectorWindow::ProcessSelectorWindow()
	{

	}

	ProcessSelectorWindow::~ProcessSelectorWindow()
	{
	}

	bool ProcessSelectorWindow::Init()
	{
		return processList.init();
	}

	void ProcessSelectorWindow::OnWindowResize(float width, float height)
	{
		// This will be called from the main loop when SDL window resizes
		ImGui::SetNextWindowSize(ImVec2(width, height));
	}

	bool ProcessSelectorWindow::RefreshProcesses()
	{
		selected_index = -1;

		selectedProcess.reset();

		return processList.Refresh();
	}

	void ProcessSelectorWindow::OnListSelect()
	{
		printf("%d ", processList.at(selected_index).processID);

		printf("%s\n", processList.at(selected_index).currentProcessName.c_str());

		selectedProcess.emplace(processList.at(selected_index));
	}

	void ProcessSelectorWindow::OnRefreshButtonPressed()
	{
		if (RefreshProcesses())
		{
			printf("Processes Refreshed with %d entries.\n", processList.size());
		}
		else
		{
			printf("Processes couldnt be refreshed");
		}
	}

	void ProcessSelectorWindow::Render()
	{
		if (ImGui::Button("Refresh"))
		{
			OnRefreshButtonPressed();
		}

		if (selectedProcess.has_value())
		{
			ImGui::Text("Selected Process: ", &selectedProcess.value().currentProcessName);
		}

		// Calculate available height for the listbox (subtract button height and some padding)
		float listbox_height = ImGui::GetContentRegionAvail().y - 30.0f; // Reserve space for text at bottom

		if (ImGui::BeginListBox("##ProcessList", ImVec2(-FLT_MIN, listbox_height)))
		{
			for (auto i = 0u; i < processList.size(); i++)
			{
				const bool is_selected = (selected_index == i);

				// this has a selectable size itd be good to make it a good size that fits text and is readable...
				// // maybe even dynamic to window size

				// Use process ID to create unique ImGui ID to avoid duplicate ID warnings
				std::string label = processList.at(i).currentProcessName + "##" + std::to_string(processList.at(i).processID);
				if (ImGui::Selectable(label.c_str(), is_selected))
				{
					selected_index = i;

					OnListSelect();
				}

				// Set the initial focus when opening the combo (scrolling to the item)
				if (is_selected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}

			ImGui::EndListBox();

			ImGui::Text("Hello from ImGui!");
		}
	}

	void ProcessSelectorWindow::RenderTest(std::optional<ProcessHandle>& attachedproc)
	{
		if (ImGui::Button("Refresh"))
		{
			OnRefreshButtonPressed();
		}

		// Calculate available height for the listbox (subtract button height and some padding)
		float listbox_height = ImGui::GetContentRegionAvail().y - 30.0f; // Reserve space for text at bottom

		if (ImGui::BeginListBox("##ProcessList", ImVec2(-FLT_MIN, listbox_height)))
		{
			for (auto i = 0u; i < processList.size(); i++)
			{
				const bool is_selected = (selected_index == i);

				// this has a selectable size itd be good to make it a good size that fits text and is readable...
				// // maybe even dynamic to window size

				// Use process ID to create unique ImGui ID to avoid duplicate ID warnings
				std::string label = processList.at(i).currentProcessName + "##" + std::to_string(processList.at(i).processID);

				if (ImGui::Selectable(label.c_str(), is_selected))
				{
					selected_index = i;

					attachedproc.reset();

					attachedproc.emplace(processList.at(i));

					if (!attachedproc->IsValid())
					{
						printf("Process Handle is invalid\n");

						attachedproc.reset();
						selected_index = -1;
						continue;
					}

					printf("Selected process: %s ", processList.at(i).currentProcessName.c_str());

					printf("Process ID: %d\n", processList.at(i).processID);
				}

				// Set the initial focus when opening the combo (scrolling to the item)
				if (is_selected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}

			ImGui::EndListBox();

			ImGui::Text("Hello from ImGui!");
		}
	}
}

