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

	///////////////////////////////////////////////////////////////////////////////////////////////////////
	///Gamepad/////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////

	float WindowsInput::GetAnalogLeftXImpl(int ID)
	{
		DWORD dwResult;
		XINPUT_STATE state;
		ZeroMemory(&state, sizeof(XINPUT_STATE));
		if (ID - 1 != -1)
		{
			dwResult = XInputGetState(ID - 1, &state);
			return ((float)state.Gamepad.sThumbLX / 32767);
		}
		else {
			for (DWORD i = 0; i < XUSER_MAX_COUNT; i++)
			{
				dwResult = XInputGetState(i, &state);
				if (dwResult == ERROR_SUCCESS)
				{
					return ((float)state.Gamepad.sThumbLX / 32767);
				}
			}
		}
		return 0.0f;
	}
	float WindowsInput::GetAnalogLeftYImpl(int ID)
	{
		DWORD dwResult;
		XINPUT_STATE state;
		ZeroMemory(&state, sizeof(XINPUT_STATE));
		if (ID - 1 != -1)
		{
			dwResult = XInputGetState(ID - 1, &state);
			return ((float)state.Gamepad.sThumbLY / 32767);
		}
		else {
			for (DWORD i = 0; i < XUSER_MAX_COUNT; i++)
			{
				dwResult = XInputGetState(i, &state);
				if (dwResult == ERROR_SUCCESS)
				{
					return ((float)state.Gamepad.sThumbLY / 32767);
				}
			}
		}
		return 0.0f;
	}
	float WindowsInput::GetAnalogRightXImpl(int ID)
	{
		DWORD dwResult;
		XINPUT_STATE state;
		ZeroMemory(&state, sizeof(XINPUT_STATE));
		if (ID - 1 != -1)
		{
			dwResult = XInputGetState(ID - 1, &state);
			return ((float)state.Gamepad.sThumbRX / 32767);
		}
		else {
			for (DWORD i = 0; i < XUSER_MAX_COUNT; i++)
			{
				dwResult = XInputGetState(i, &state);
				if (dwResult == ERROR_SUCCESS)
				{
					return ((float)state.Gamepad.sThumbRX / 32767);
				}
			}
		}
		return 0.0f;
	}
	float WindowsInput::GetAnalogRightYImpl(int ID)
	{
		DWORD dwResult;
		XINPUT_STATE state;
		ZeroMemory(&state, sizeof(XINPUT_STATE));
		if (ID - 1 != -1)
		{
			dwResult = XInputGetState(ID - 1, &state);
			return ((float)state.Gamepad.sThumbRY / 32767);
		}
		else {
			for (DWORD i = 0; i < XUSER_MAX_COUNT; i++)
			{
				dwResult = XInputGetState(i, &state);
				if (dwResult == ERROR_SUCCESS)
				{
					return ((float)state.Gamepad.sThumbRY / 32767);
				}
			}
		}
		return 0.0f;
	}
	bool WindowsInput::IsGamepadButtonPressedImpl(int ID, int code)
	{
		DWORD dwResult;
		XINPUT_STATE state;
		ZeroMemory(&state, sizeof(XINPUT_STATE));
		if (ID-1 != -1)
		{
			dwResult = XInputGetState(ID - 1, &state);
			return (state.Gamepad.wButtons & code);
		}
		else {
			for (DWORD i = 0; i < XUSER_MAX_COUNT; i++)
			{
				dwResult = XInputGetState(i, &state);
				if (dwResult == ERROR_SUCCESS)
				{
					return (state.Gamepad.wButtons & code);
				}
			}
		}
		return false;
	}
	bool WindowsInput::IsGamepadButtonJustPressedImpl(int ID, int code)
	{
		static bool repeat = false;
		bool b;
		DWORD dwResult;
		XINPUT_STATE state;
		ZeroMemory(&state, sizeof(XINPUT_STATE));
		if (ID - 1 != -1) {
			dwResult = XInputGetState(ID-1, &state);
			b = (state.Gamepad.wButtons & XINPUT_GAMEPAD_A);
			if (!b)
				repeat = false;
			if (repeat) {
				return false;
			}
			repeat = b;
			return repeat;
		}
		else {
			for (DWORD i = 0; i < XUSER_MAX_COUNT; i++)
			{
				dwResult = XInputGetState(i, &state);
				if (dwResult == ERROR_SUCCESS)
				{
					b = (state.Gamepad.wButtons & XINPUT_GAMEPAD_A);
					if (!b)
						repeat = false;
					if (repeat) {
						return false;
					}
					repeat = b;
					return repeat;
				}
			}
		}
		return false;
	}
	void WindowsInput::VibrateControllerImpl(int ID, int left, int right)
	{
		XINPUT_VIBRATION vibrationDesc;

		ZeroMemory(&vibrationDesc, sizeof(XINPUT_VIBRATION));

		vibrationDesc.wLeftMotorSpeed = left;
		vibrationDesc.wRightMotorSpeed = right;
		XInputSetState(ID, &vibrationDesc);

	}
}