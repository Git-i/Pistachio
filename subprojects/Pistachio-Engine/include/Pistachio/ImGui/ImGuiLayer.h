#pragma once
#include "Pistachio/Platform/Windows/WindowsWindow.h"
#include "Pistachio/Core/Layer.h"
#include "../vendor/imgui/imgui.h"
#include "../vendor/imgui/imgui_impl_dx11.h"
#include "../vendor/imgui/imgui_impl_win32.h"
#include "Pistachio/Event/KeyEvent.h"
#include "Pistachio/Event/Event.h"

namespace Pistachio {
	class PISTACHIO_API ImGuiLayer : public Layer {
	public:
		ImGuiLayer();
		~ImGuiLayer();
		void OnUpdate(float delta) override;
		void OnEvent(Event& event) override;
		void Begin();
		void End();
		bool OnKeyPressed(KeyPressedEvent& event);
		void OnAttach() override;
		void SetGreenTheme();
		void SetDarkTheme();
		void SetLightTheme();
		bool BlockEvents;
	private:
	};
}