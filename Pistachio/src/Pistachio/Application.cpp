#include "ptpch.h"
#include "Pistachio/Log.h"
#include "Application.h"
#include "Pistachio/Event/Event.h"
namespace Pistachio {
	Application::Application()
	{
		#define BIND_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)
		void* n = GetWindowDataPtr();
		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));
	}
	Application::~Application()
	{
	}
	void Application::OnEvent(Event& e)
	{
		PT_CORE_INFO("{0}", e);
	}
		
	void Application::Run()
	{
		m_Window->OnUpdate();
	}
}