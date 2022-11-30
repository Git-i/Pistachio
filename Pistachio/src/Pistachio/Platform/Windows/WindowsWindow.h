#pragma once

#include "Pistachio/Window.h"
#include "GLFW/glfw3.h"
void* WindowDataPtr = new void*;
void* GetWindowDataPtr()
{
return WindowDataPtr;
}
void SetWindowDataPtr(void* value)
{
	WindowDataPtr = value;
}
struct WindowData
{
	unsigned int width;
	unsigned int height;
	const wchar_t* title;
	bool vsync;
	EventCallbackFn EventCallback;
};

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
		static void resize(int width, int height);
		inline void SetEventCallback(const EventCallbackFn& event) { m_data.EventCallback = event; };
	private:
		virtual int Init(const WindowInfo& info, HINSTANCE hInstance);
		virtual void Shutdown();
		
		WindowData m_data;
	};
}
