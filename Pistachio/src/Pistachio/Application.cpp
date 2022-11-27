#include "ptpch.h"
#include "GLFW/glfw3.h"

#include "Application.h"
#include "Pistachio/Event/KeyEvent.h"
namespace Pistachio {
	Application::Application()
	{
		m_Window = std::unique_ptr<Window>(Window::Create());
	}
	Application::~Application()
	{
	}
	void Application::Run()
	{
		std::cout << "info.title";
		while (m_Running)
		{
			glClearColor(1.0f, 0.0f, 0.2f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);
			m_Window->OnUpdate();
		}
	}
}