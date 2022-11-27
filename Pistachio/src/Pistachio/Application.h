#pragma once

#include "Core.h" 
#include "Pistachio/Event/Event.h"
#include "Pistachio/Window.h"

namespace Pistachio {

	class PISTACHIO_API Application
	{
	public:
		Application();
		virtual ~Application();
		void Run();
	private:
		std::unique_ptr<Window> m_Window;
		bool m_Running = true;
	};

	Application* CreateApplication();
}
