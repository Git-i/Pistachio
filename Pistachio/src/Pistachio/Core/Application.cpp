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
		m_Window = Scope<Window>(Window::Create());
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
		//EventDispatcher dispatcher(e);
		PT_CORE_TRACE("{0}", e);
		for (auto it = m_layerstack.end(); it != m_layerstack.begin(); )
		{
			(*--it)->OnEvent(e);
			if (e.Handled)
				break;
		}
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
			m_Window->OnUpdate(delta);
			for (Layer* layer : m_layerstack)
				layer->OnUpdate(delta);
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