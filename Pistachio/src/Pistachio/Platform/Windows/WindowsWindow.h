#pragma once

#include "Pistachio/Window.h"
#include "GLFW/glfw3.h"

namespace Pistachio {
	class WindowsWindow : public Pistachio::Window
	{
	public:
		WindowsWindow(const WindowInfo& info);
		virtual ~WindowsWindow();

		void OnUpdate() override;
		inline unsigned int GetWidth() const override { return m_data.width; }
		inline unsigned int GetHeight() const override { return m_data.height; }

		void SetVsync(bool enabled) override;
		bool IsVsync() const override;
	private:
		GLFWwindow* m_window;
		virtual void Init(const WindowInfo& info);
		virtual void Shutdown();
		WindowInfo m_data;
	};
}
