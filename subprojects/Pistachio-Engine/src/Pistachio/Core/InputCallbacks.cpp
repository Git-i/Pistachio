#include "ptpch.h"
#include "InputCallbacks.h"
bool KeyRepeat = false;
int Pistachio::LastKey = 0;
void Pistachio::OnMouseScroll(float xamount, float yamount)
{
	WindowData& data = *(WindowData*)GetWindowDataPtr();
	MouseScrolledEvent event(xamount, yamount);
	if (data.EventCallback != NULL) {
		data.EventCallback(event);
	}
}

void Pistachio::OnResize(unsigned int width, unsigned int height)
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
	if (code == Pistachio::LastKey)
		KeyRepeat = true;
	else
		KeyRepeat = false;
	Pistachio::LastKey = code;
}
void Pistachio::OnKeyUp(int code)
{
	WindowData& data = *(WindowData*)GetWindowDataPtr();
	Pistachio::KeyReleasedEvent event(code);
	if (data.EventCallback != NULL) {
		data.EventCallback(event);
	}
	KeyRepeat = false;
	Pistachio::KeyRepeatPoll = false;
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

void Pistachio::OnMouseMove(int x, int y)
{
	WindowData& data = *(WindowData*)GetWindowDataPtr();
	Pistachio::MouseMovedEvent event(x, y);
	if (data.EventCallback != NULL) {
		data.EventCallback(event);
	}
}
