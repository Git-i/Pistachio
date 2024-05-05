#include "ptpch.h"
#include "Pistachio/Core/Input.h"
#include "Pistachio/Core/Application.h"

namespace Pistachio {
	bool KeyRepeatPoll;
	int LastKeyPoll;
	bool Input::IsKeyPressed(KeyCode code)
	{
		return (::GetKeyState(code) & 0x8000) != 0;
	}
	bool Input::IsKeyJustPressed(KeyCode code)
	{
		bool a = 0;
		bool first = 1;
		if (GetActiveWindow() == Application::Get().GetWindow().pd.hwnd)
			a = ((::GetKeyState(code) & 0x8000) != 0) && (!(KeyRepeatPoll && LastKeyPoll == code));
		if (first == 1) {
			LastKeyPoll = code;
			first = 0;
		}
		if (GetKeyState(code)) {
			if (code == Pistachio::LastKeyPoll)
				KeyRepeatPoll = true;
			else
				KeyRepeatPoll = false;
			LastKeyPoll = code;
		}
		return a;
	}
	int Input::GetMouseX(bool wndcoord)
	{
		POINT Position;
		GetCursorPos(&Position);
		if (wndcoord)
			ScreenToClient(Application::Get().GetWindow().pd.hwnd, &Position);
		return Position.x;
	}
	int Input::GetMouseY(bool wndcoord)
	{
		POINT Position;
		GetCursorPos(&Position);
		if (wndcoord)
			ScreenToClient(Application::Get().GetWindow().pd.hwnd, &Position);
		return Position.y;
	}

	bool Input::IsMouseButtonPressed(MouseButton button)
	{
		return Input::IsKeyPressed(button);
	}
	bool Input::IsMouseButtonJustPressed(MouseButton button)
	{
		return Input::IsKeyJustPressed(button);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////
	///Gamepad/////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////

	float Input::GetLeftAnalogX(int ID)
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
	float Input::GetLeftAnalogY(int ID)
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
	float Input::GetRightAnalogX(int ID)
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
	float Input::GetRightAnalogY(int ID)
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
	bool Input::IsGamepadButtonPressed(int ID, int code)
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
	bool Input::IsGamepadButtonJustPressed(int ID, int code)
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
	void Input::VibrateController(int ID, int left, int right)
	{
		XINPUT_VIBRATION vibrationDesc;

		ZeroMemory(&vibrationDesc, sizeof(XINPUT_VIBRATION));

		vibrationDesc.wLeftMotorSpeed = left;
		vibrationDesc.wRightMotorSpeed = right;
		XInputSetState(ID - 1, &vibrationDesc);

	}
	float Input::GetLeftTriggerState(int ID)
	{
		DWORD dwResult;
		XINPUT_STATE state;
		ZeroMemory(&state, sizeof(XINPUT_STATE));
		if (ID - 1 != -1)
		{
			dwResult = XInputGetState(ID - 1, &state);
			return (state.Gamepad.bLeftTrigger);
		}
		else {
			for (DWORD i = 0; i < XUSER_MAX_COUNT; i++)
			{
				dwResult = XInputGetState(i, &state);
				if (dwResult == ERROR_SUCCESS)
				{
					return (state.Gamepad.bLeftTrigger);
				}
			}
		}
		return 0.0f;
	}
	float Input::GetRightTriggerState(int ID)
	{
		DWORD dwResult;
		XINPUT_STATE state;
		ZeroMemory(&state, sizeof(XINPUT_STATE));
		if (ID - 1 != -1)
		{
			dwResult = XInputGetState(ID - 1, &state);
			return (state.Gamepad.bRightTrigger);
		}
		else {
			for (DWORD i = 0; i < XUSER_MAX_COUNT; i++)
			{
				dwResult = XInputGetState(i, &state);
				if (dwResult == ERROR_SUCCESS)
				{
					return (state.Gamepad.bRightTrigger);
				}
			}
		}
		return 0.0f;
	}
}