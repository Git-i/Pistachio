#pragma once

#include "Pistachio/Core.h"
#include "Pistachio/Core/LayerStack.h"
#include "Pistachio/Event/Event.h"
#include "Pistachio/Core/Window.h"
#include "Pistachio/Event/KeyEvent.h"

#include "Pistachio/Renderer/Renderer.h"
#include "Pistachio/Renderer/Buffer.h"
#include "Pistachio/Renderer/Shader.h"
#include "Pistachio/Renderer/Camera.h"
namespace Pistachio {

	struct PISTACHIO_API InitModeHeadless
	{

	};
	class PISTACHIO_API Application
	{
	public:
		Application(const char* name);
		Application(const char* name, InitModeHeadless headless);
		virtual ~Application();
		void Run();
		void OnEvent(Event& e);
		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);
		bool OnWindowResize(WindowResizeEvent& e);
		static Application& Get();
		inline Window& GetWindow() { return *m_Window; }
		void SetImGuiContext(void* ctx);
	private:
		bool m_headless = false;
		LayerStack m_layerstack;
		std::unique_ptr<Window> m_Window;
		bool m_Running = true;
		bool m_minimized = false;
		static Application* s_Instance;
		LARGE_INTEGER frequency;
		double period;
		double lastFrameTime = 0.0f;
		double InitTime;
		LARGE_INTEGER ticks;
	};
	
	Application* CreateApplication();
}
