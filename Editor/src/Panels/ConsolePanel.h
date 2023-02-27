#pragma once
#include "imgui_console/imgui_console.h"
namespace Pistachio {
	class ConsolePanel {
	public:
		ConsolePanel();
		void OnImGuiRender();
		bool activated = true;
	private:
		ImGuiConsole console;
	};
}
