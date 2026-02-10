#pragma once

#include <SDL3/SDL.h>
#include <stdio.h>
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
#include "imgui_stdlib.h"
#include <vector>
#include <string>
#include <optional>
#include "ProcessList.h"
#include "ProcessHandle.h"


namespace mem::processselector
{
	class ProcessSelectorWindow
	{
	public:
		ProcessSelectorWindow();
		~ProcessSelectorWindow();

		bool Init();

		void OnWindowResize(float width, float height);

		void OnListSelect();

		void OnRefreshButtonPressed();

		bool RefreshProcesses();

		void Render();

		void RenderTest(std::optional<ProcessHandle>& attachedproc);

	private:
		ProcessList processList;

		int selected_index = -1;

		std::optional<Process> selectedProcess;

		std::optional<ProcessHandle> attachedProcess;
	};
}



