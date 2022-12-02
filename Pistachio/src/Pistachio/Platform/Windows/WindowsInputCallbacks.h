#pragma once
#include "Pistachio/Event/ApplicationEvent.h"
#include "Pistachio/Event/KeyEvent.h"
#include "Pistachio/Event/MouseEvent.h"
bool KeyRepeat = false;
namespace Pistachio {
	int LastKey = 0;
	void OnMousseScroll(float xamount, float yamount)
	{
		WindowData& data = *(WindowData*)GetWindowDataPtr();
		MouseScrolledEvent event(xamount, yamount);
		if (data.EventCallback != NULL) {
			data.EventCallback(event);
		}
	}
	void OnResize(int width, int height)
	{
		WindowData& data = *(WindowData*)GetWindowDataPtr();
		data.width = width;
		data.height = height;
		Pistachio::WindowResizeEvent event(width, height);
		if (data.EventCallback != NULL) {
			data.EventCallback(event);
		}

	}
	void OnKeyDown(int code)
	{
		WindowData& data = *(WindowData*)GetWindowDataPtr();
		Pistachio::KeyPressedEvent event(code, KeyRepeat);
		if (data.EventCallback != NULL) {
			data.EventCallback(event);
		}
	}
	void OnMouseButtonPress(int button)
	{
		WindowData& data = *(WindowData*)GetWindowDataPtr();
		Pistachio::MouseButtonPressedEvent event(button);
		if (data.EventCallback != NULL) {
			data.EventCallback(event);
		}
	}
	void OnMouseButtonRelease(int button)
	{
		WindowData& data = *(WindowData*)GetWindowDataPtr();
		Pistachio::MouseButtonReleasedEvent event(button);
		if (data.EventCallback != NULL) {
			data.EventCallback(event);
		}
	}
}