#include "ptpch.h"
#include "WindowsWindow.h"
#include "Pistachio/Log.h"

namespace Pistachio {
	static bool s_GLFWInitialized = false;
	Window* Window::Create(const WindowInfo& info)
	{
		return new WindowsWindow(info);
	}
	WindowsWindow::WindowsWindow(const WindowInfo& info)
	{
		Init(info);
	}
	WindowsWindow::~WindowsWindow()
	{
		Shutdown();
	}
	void WindowsWindow::Init(const WindowInfo& info)
	{
		m_data.title = info.title;
		m_data.height = info.height;
		m_data.width = info.width;
		m_data.vsync = info.vsync;
		PT_CORE_INFO("Creating Window {0} ({1}, {2})", info.title, info.width, info.height);

		if (!s_GLFWInitialized)
		{
			if (!glfwInit())
			{
				PT_CORE_ERROR("Failed to initialize glfw");
				__debugbreak();
			}
			s_GLFWInitialized = true;
		}
		m_window = glfwCreateWindow(info.width, info.height, info.title, NULL, NULL);
		glfwMakeContextCurrent(m_window);
		glfwSetWindowUserPointer(m_window, &m_data);
		SetVsync(info.vsync);
	}
	void WindowsWindow::Shutdown()
	{
		glfwDestroyWindow(m_window);
	}
	void WindowsWindow::OnUpdate()
	{
		glfwPollEvents();
		glfwSwapBuffers(m_window);
	}
	void WindowsWindow::SetVsync(bool enabled)
	{
		if (enabled)
			glfwSwapInterval(0);
		else
			glfwSwapInterval(1);
		m_data.vsync = enabled;
	}
	bool WindowsWindow::IsVsync() const
	{
		return m_data.vsync;
	}
}
