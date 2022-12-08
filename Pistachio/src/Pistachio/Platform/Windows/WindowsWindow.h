#pragma once

#include "Pistachio/Core/Window.h"

extern void* WindowDataPtr;
void* GetWindowDataPtr();
void SetWindowDataPtr(void* value);



namespace Pistachio {
	class WindowsWindow : public Window
	{
	public:
		WindowsWindow(const WindowInfo& info);
		virtual ~WindowsWindow();

		void OnUpdate() override;
		inline unsigned int GetWidth() const override { return m_data.width; }
		inline unsigned int GetHeight() const override { return m_data.height; }
		void EndFrame() const override;
		void SetVsync(bool enabled) override;
		bool IsVsync() const override;
		inline void SetEventCallback(const EventCallbackFn& event) { m_data.EventCallback = event; }
	private:
		virtual int Init(const WindowInfo& info, HINSTANCE hInstance);
		virtual void Shutdown();
	};
}
