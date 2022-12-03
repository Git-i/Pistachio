#include "ptpch.h"
#include "WindowsInputCallbacks.h"
bool KeyRepeat = false;
int Pistachio::LastKey = 0;
void Pistachio::OnMousseScroll(float xamount, float yamount)
{
	WindowData& data = *(WindowData*)GetWindowDataPtr();
	MouseScrolledEvent event(xamount, yamount);
	if (data.EventCallback != NULL) {
		data.EventCallback(event);
	}
}

void Pistachio::OnResize(int width, int height)
{
	WindowData& data = *(WindowData*)GetWindowDataPtr();
	data.width = width;
	data.height = height;
	Pistachio::WindowResizeEvent event(width, height);
	if (data.EventCallback != NULL) {
		data.EventCallback(event);
	}

}

void Pistachio::OnKeyDown(int code)
{
	WindowData& data = *(WindowData*)GetWindowDataPtr();
	Pistachio::KeyPressedEvent event(code, KeyRepeat && LastKey == code);
	if (data.EventCallback != NULL) {
		data.EventCallback(event);
	}
}

void Pistachio::OnMouseButtonPress(int button)
{
	WindowData& data = *(WindowData*)GetWindowDataPtr();
	Pistachio::MouseButtonPressedEvent event(button);
	if (data.EventCallback != NULL) {
		data.EventCallback(event);
	}
}

void Pistachio::OnMouseButtonRelease(int button)
{
	WindowData& data = *(WindowData*)GetWindowDataPtr();
	Pistachio::MouseButtonReleasedEvent event(button);
	if (data.EventCallback != NULL) {
		data.EventCallback(event);
	}
}
