#pragma once

#include "Pistachio/Core.h"
#include "Pistachio/Core/LayerStack.h"
#include "Pistachio/Event/Event.h"
#include "Pistachio/Core/Window.h"

namespace Pistachio {

	class PISTACHIO_API Application
	{
	public:
		Application();
		virtual ~Application();
		void Run();
		void OnEvent(Event& e);
		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);

		inline static Application& Get() { return *s_Instance; }
		inline Window& GetWindow() { return *m_Window; }
	private:
		LayerStack m_layerstack;
		std::unique_ptr<Window> m_Window;
		bool m_Running = true;
		static Application* s_Instance;
	};

	Application* CreateApplication();
}
