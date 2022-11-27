#pragma once

#include "ptpch.h"
#include "Pistachio/Core.h"

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
		virtual bool IsVsync() const = 0;
		static Window* Create(const WindowInfo& info = WindowInfo());
	};
}
