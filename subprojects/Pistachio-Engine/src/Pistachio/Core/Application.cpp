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
	Application::Application(const char* name, ApplicationOptions opt)
	{
		PT_PROFILE_FUNCTION();
		s_Instance = this;
		m_headless = opt.headless;
		Pistachio::Log::Init();
		RendererBase::InitOptions ropt;
		ropt.headless = opt.headless;
		ropt.luid = opt.gpu_luid;
		if (!opt.headless)
		{
			WindowInfo info;
			info.height = 720;
			info.width = 1280;
			info.vsync = 1;
			info.title = name;
			m_Window = Scope<Window>(Window::Create(info));
			m_Window->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));
			RendererBase::Init(&m_Window->pd, ropt);
		}
		else RendererBase::Init(NULL, ropt);
		Renderer::Init("resources/textures/hdr/Alexs_Apt_2k.hdr");
		Renderer2D::Init();
		std::cout << "phys time" << std::endl;
		//Physics::Init();
		std::cout << "physics initialized" << std::endl;
#ifdef IMGUI
		PushOverlay(m_ImGuiLayer);
#endif
		InitTime = std::chrono::high_resolution_clock::now();
		lastFrameTime = std::chrono::milliseconds(0);
		//QueryPerformanceFrequency(&frequency);
		//period = 1 / (float)frequency.QuadPart;
		//QueryPerformanceCounter(&ticks);
		//InitTime = (ticks.QuadPart * period);
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
	Application& Application::Get()
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
		
		while (m_Running) {
			auto now = std::chrono::high_resolution_clock::now();
			std::chrono::milliseconds time = std::chrono::duration_cast<std::chrono::milliseconds>(now - InitTime);
			std::chrono::duration<float,std::milli> delta;
			delta = time - lastFrameTime;
			lastFrameTime = time;
			FrameMark;
			
			if (!m_Running)
				break;


			m_Window->OnUpdate(delta.count());
			if (!m_minimized) {
				for (Layer* layer : m_layerstack)
					layer->OnUpdate(delta.count());
			}
			for (Layer* layer : m_layerstack)
				layer->OnImGuiRender();
			Renderer::EndScene();
		}
		
	}

	void Application::Step()
	{
		//QueryPerformanceCounter(&ticks);
		//double time = (ticks.QuadPart * period) - InitTime;
		//float delta = (float)(time - lastFrameTime);
		FrameMark;
		double delta = 0.016;
		//lastFrameTime = time;
		for (Layer* layer : m_layerstack)
			layer->OnUpdate(delta);
		for (Layer* layer : m_layerstack)
			layer->OnImGuiRender();
		Renderer::EndScene();
		return;
	}
	
}