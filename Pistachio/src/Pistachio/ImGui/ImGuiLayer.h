#pragma once
#include "Pistachio/Platform/Windows/WindowsWindow.h"
#include "Pistachio/Core/Layer.h"


namespace Pistachio {
	class PISTACHIO_API ImGuiLayer : public Layer {
	public:
		ImGuiLayer();
		~ImGuiLayer();

		void OnUpdate() override;
		void OnEvent(Event& event) override;
		void OnAttach() override;
	private:

	};
}