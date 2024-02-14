#include "ptpch.h"
#include "Pistachio/Core/Log.h"
#include "Pistachio/Core/Application.h"
#include "Pistachio/Event/Event.h"
#include "Pistachio/Core/KeyCodes.h"
#include "Pistachio/Core/Input.h"
#include "Pistachio/Physics/Physics.h"
#include "Pistachio/Renderer/Renderer2D.h"
#include "Tracy.hpp"
#include "imgui.h"
namespace Pistachio {
	Application* Application::s_Instance = nullptr;
	Application::Application(const char* name)
	{
		PT_PROFILE_FUNCTION();
		s_Instance = this;
		WindowInfo info;
		info.height = 720;
		info.width = 1280;
		info.vsync = 1;
		info.title = name;
		m_Window = Scope<Window>(Window::Create(info));
		Pistachio::Log::Init();
		RendererBase::Init(m_Window->pd.hwnd);
		Renderer::Init("resources/textures/hdr/Alexs_Apt_2k.hdr");
		Renderer2D::Init();
		std::cout << "phys time" << std::endl;
		//Physics::Init();
		std::cout << "physics initialized" << std::endl;
		m_Window->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));
#ifdef IMGUI
		PushOverlay(m_ImGuiLayer);
#endif
		QueryPerformanceFrequency(&frequency);
		period = 1 / (float)frequency.QuadPart;
		QueryPerformanceCounter(&ticks);
		InitTime = (ticks.QuadPart * period);
	}

	Application::Application(const char* name, InitModeHeadless headless)
	{
		PT_PROFILE_FUNCTION();
		s_Instance = this;
		m_headless = true;
		WindowInfo info;
		info.height = 1;
		info.width = 1;
		info.vsync = 0;
		info.title = name;
		m_Window = Scope<Window>(Window::Create(info, m_headless));
		Pistachio::Log::Init();
		RendererBase::Init(m_Window->pd.hwnd);
		Renderer::Init("resources/textures/hdr/paranoma_image.png");
		Renderer2D::Init();
		std::cout << "phys time" << std::endl;
		//Physics::Init();
		std::cout << "physics initialized" << std::endl;
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
	inline Application& Application::Get()
	{
		return *s_Instance; 
	}
	void Application::SetImGuiContext(void* ctx)
	{
		ImGui::SetCurrentContext((ImGuiContext*)ctx);
	}
	void Application::Run()
	{
		PT_PROFILE_FUNCTION();
		if (m_headless)
		{
			QueryPerformanceCounter(&ticks);
			double time = (ticks.QuadPart * period) - InitTime;
			float delta = (float)(time - lastFrameTime);
			FrameMark;
			lastFrameTime = time;
			if (!m_minimized) {
				for (Layer* layer : m_layerstack)
					layer->OnUpdate(delta);
			}
			for (Layer* layer : m_layerstack)
				layer->OnImGuiRender();
			Renderer::EndScene();
			return;
		}
		while (m_Running) {
			QueryPerformanceCounter(&ticks);
			double time = (ticks.QuadPart * period) - InitTime;
			float delta = (float)(time - lastFrameTime);
			FrameMark;
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
			if (!m_minimized) {
				for (Layer* layer : m_layerstack)
					layer->OnUpdate(delta);
			}
			for (Layer* layer : m_layerstack)
				layer->OnImGuiRender();
			Renderer::EndScene();
		}
		
	}
	
}