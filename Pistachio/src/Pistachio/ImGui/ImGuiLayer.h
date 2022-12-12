#pragma once
#include "Pistachio/Platform/Windows/WindowsWindow.h"
#include "Pistachio/Core/Layer.h"
#include "../vendor/imgui/imgui.h"
#include "../vendor/imgui/imgui_impl_dx11.h"
#include "../vendor/imgui/imgui_impl_win32.h"

namespace Pistachio {
	class PISTACHIO_API ImGuiLayer : public Layer {
	public:
		ImGuiLayer();
		~ImGuiLayer();

		void OnUpdate(float delta) override;
		void OnEvent(Event& event) override;
		void OnAttach() override;
	private:

	};
}