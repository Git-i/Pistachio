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
	struct WindowData
	{
		unsigned int width = 0;
		unsigned int height =0;
		const char* title;
		bool vsync = true;
		EventCallbackFn EventCallback;
	};

	namespace Pistachio {

		struct WindowInfo
		{
			unsigned int width;
			unsigned int height;
			const char* title;
			bool vsync;
			WindowInfo(unsigned int w = 1280, unsigned int h = 720, const char* t = "Pistachio Engine", bool Vsync = true)
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
		protected:
			WindowData m_data;
		};
		
	}

