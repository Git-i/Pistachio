#include "ptpch.h"
#include "Pistachio/Core/Log.h"
#include "Pistachio/Core/Application.h"
#include "Pistachio/Event/Event.h"
#include "Pistachio/Core/KeyCodes.h"
#include "Pistachio/ImGui/ImGuiLayer.h"
#include "Pistachio/Core/Input.h"

namespace Pistachio {

	Application* Application::s_Instance = nullptr;
	Application::Application()
	{
		s_Instance = this;
		void* n = GetWindowDataPtr();
		WindowInfo info;
		info.height = 720;
		info.width = 1280;
		info.vsync = 0;
		info.title = "LOLOA";
		m_Window = Scope<Window>(Window::Create(info));
		m_Window->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));
		PushOverlay(new Pistachio::ImGuiLayer());
		QueryPerformanceFrequency(&frequency);
		period = 1 / (float)frequency.QuadPart;
		QueryPerformanceCounter(&ticks);
		InitTime = (ticks.QuadPart * period);
	}

	Application::~Application()
	{
	}

	void Application::PushLayer(Layer* layer)
	{
		m_layerstack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* overlay)
	{
		m_layerstack.PushOverlay(overlay);
		overlay->OnAttach();
	}

	void Application::OnEvent(Event& e)
	{
		
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(Application::OnWindowResize));
		Renderer::OnEvent(e);
		PT_CORE_TRACE("{0}", e);
		for (auto it = m_layerstack.end(); it != m_layerstack.begin(); )
		{
			(*--it)->OnEvent(e);
			if (e.Handled)
				break;
		}
	}
	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		if (e.GetWidth() == 0)
			m_minimized = true;
		else
			m_minimized = false;
		return false;
	}
	void Application::Run()
	{
		while (m_Running) {
			QueryPerformanceCounter(&ticks);
			double time = (ticks.QuadPart * period) - InitTime;
			float delta = (float)(time - lastFrameTime);
			lastFrameTime = time;
			MSG msg = {};
			while (PeekMessageW(&msg, 0, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessageW(&msg);
				if (msg.message == WM_QUIT)
					m_Running = false;
			}
			if (!m_Running)
				break;
			// Start the Dear ImGui frame
			ImGui_ImplDX11_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
			m_Window->OnUpdate(delta);
			if (!m_minimized) {
				for (Layer* layer : m_layerstack)
					layer->OnUpdate(delta);
			}
			for (Layer* layer : m_layerstack)
				layer->OnImGuiRender();
			ImGui::Render();
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
			if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			{
				ImGui::UpdatePlatformWindows();
				ImGui::RenderPlatformWindowsDefault();
			}
			Renderer::EndScene();
		}
		
	}
	
}