#include "ptpch.h"#
#include "Pistachio/Core/Log.h"
#include "Pistachio/Core/Application.h"
#include "Pistachio/Event/Event.h"
#include "Pistachio/Core/KeyCodes.h"
#include "Pistachio/Core/Input.h"
#include "Pistachio/Physics/Physics.h"
#include "Pistachio/Renderer/Renderer2D.h"
namespace Pistachio {
	Application* Application::s_Instance = nullptr;
	Application::Application(const char* name)
	{
		PT_PROFILE_FUNCTION();
		m_ImGuiLayer = new ImGuiLayer;
		s_Instance = this;
		WindowInfo info;
		info.height = 720;
		info.width = 1280;
		info.vsync = 0;
		info.title = name;
		m_Window = Scope<Window>(Window::Create(info));
		Pistachio::Log::Init();
		RendererBase::Init(m_Window->pd.hwnd);
		Renderer::Init("resources/textures/hdr/panorama_image.png");
		Renderer2D::Init();
		Physics::Init();
		m_Window->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));
#ifdef IMGUI
		PushOverlay(m_ImGuiLayer);
#endif
		QueryPerformanceFrequency(&frequency);
		period = 1 / (float)frequency.QuadPart;
		QueryPerformanceCounter(&ticks);
		InitTime = (ticks.QuadPart * period);
	}

	Application::~Application()
	{
		Pistachio::Renderer::Shutdown();
		Pistachio::Physics::Shutdown();
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
		for (auto it = m_layerstack.end(); it != m_layerstack.begin(); )
		{
			(*--it)->OnEvent(e);
			if (e.Handled)
				break;
		}
	}
	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		PT_PROFILE_FUNCTION()
		if (e.GetWidth() == 0)
			m_minimized = true;
		else
		{
			RendererBase::Resize(e.GetWidth(), e.GetHeight());
			m_minimized = false;
		}
		return false;
	}
	void Application::Run()
	{
		PT_PROFILE_FUNCTION()
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
			m_ImGuiLayer->Begin();


			m_Window->OnUpdate(delta);
			if (!m_minimized) {
				for (Layer* layer : m_layerstack)
					layer->OnUpdate(delta);
			}
			for (Layer* layer : m_layerstack)
				layer->OnImGuiRender();
			m_ImGuiLayer->End();
			Renderer::EndScene();
		}
		
	}
	
}