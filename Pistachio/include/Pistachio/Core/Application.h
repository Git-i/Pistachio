#pragma once

#include "Pistachio/Core.h"
#include "Pistachio/Core/LayerStack.h"
#include "Pistachio/Event/Event.h"
#include "Pistachio/Core/Window.h"
#include "Pistachio/Event/KeyEvent.h"

#include "Pistachio/ImGui/ImGuiLayer.h"
#include "Pistachio/Renderer/Renderer.h"
#include "Pistachio/Renderer/Buffer.h"
#include "Pistachio/Renderer/Shader.h"
#include "Pistachio/Renderer/Camera.h"
namespace Pistachio {

	class PISTACHIO_API Application
	{
	public:
		Application(const char* name);
		virtual ~Application();
		void Run();
		void OnEvent(Event& e);
		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);
		bool OnWindowResize(WindowResizeEvent& e);
		inline static Application& Get() { return *s_Instance; }
		inline Window& GetWindow() { return *m_Window; }
		inline ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer; }
	private:
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
		ImGuiLayer* m_ImGuiLayer;
	};

	Application* CreateApplication();
}
