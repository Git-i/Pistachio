#pragma once

#include "ptpch.h"
#include "Pistachio/Core.h"
#include "Pistachio/Event/Event.h"

void SetWindowDataPtr(void* value);
void* GetWindowDataPtr();


	struct PlatformData {
		#ifdef PT_PLATFORM_WINDOWS
		HWND hwnd = NULL;
		#endif // PT_PLATFORM_WINDOWS
	};

using EventCallbackFn = std::function<void(Pistachio::Event& e)>;
namespace Pistachio {

	struct WindowInfo
	{
		unsigned int width;
		unsigned int height;
		const wchar_t* title;
		bool vsync;
		WindowInfo(unsigned int w = 1280, unsigned int h = 720, const wchar_t* t = L"Pistachio Engine", bool Vsync = true)
			: width(w), height(h), title(t), vsync(Vsync) {}
	};
	class PISTACHIO_API Window
	{
	public:
		
		virtual ~Window() {}
		virtual void OnUpdate() = 0;

		virtual unsigned int GetWidth() const = 0;
		virtual unsigned int GetHeight() const = 0;

		virtual void SetVsync(bool enabled) = 0;
		virtual void SetEventCallback(const EventCallbackFn& event) = 0;
		virtual bool IsVsync() const = 0;
		static Window* Create(const WindowInfo& info = WindowInfo());
		PlatformData pd;
	};
}
