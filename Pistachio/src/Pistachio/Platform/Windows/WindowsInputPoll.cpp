#include "ptpch.h"
#include "WindowsInputPoll.h"

namespace Pistachio {
	Input* Input::s_Instance = new WindowsInput;
	bool WindowsInput::IsKeyPressedImpl(KeyCode code)
	{
		if(GetActiveWindow() == Application::Get().GetWindow().pd.hwnd)
		return GetAsyncKeyState(code);
		return false;
	}
	bool WindowsInput::IsKeyJustPressedImpl(KeyCode code)
	{
		bool a = 0;
		bool first = 1;
		if (GetActiveWindow() == Application::Get().GetWindow().pd.hwnd)
			a = (GetAsyncKeyState(code) && (!(KeyRepeatPoll && LastKeyPoll == code)));
		if (first == 1) {
			LastKeyPoll = code;
			first = 0;
		}
		if (GetAsyncKeyState(code)) {
			if (code == Pistachio::LastKeyPoll)
				KeyRepeatPoll = true;
			else
				KeyRepeatPoll = false;
			LastKeyPoll = code;
		}
		return a;
	}
	int WindowsInput::GetMouseXImpl(bool wndcoord)
	{
		POINT Position;
		GetCursorPos(&Position);
		if (wndcoord)
			ScreenToClient(Application::Get().GetWindow().pd.hwnd, &Position);
		return Position.x;
	}
	int WindowsInput::GetMouseYImpl(bool wndcoord)
	{
		POINT Position;
		GetCursorPos(&Position);
		if (wndcoord)
			ScreenToClient(Application::Get().GetWindow().pd.hwnd, &Position);
		return Position.y;
	}
}