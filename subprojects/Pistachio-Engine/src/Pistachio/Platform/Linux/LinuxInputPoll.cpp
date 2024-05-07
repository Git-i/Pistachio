#include "ptpch.h"
#include "Pistachio/Core/Input.h"
#include "Pistachio/Core/Application.h"

namespace Pistachio {
	bool KeyRepeatPoll;
	int LastKeyPoll;
	bool Input::IsKeyPressed(KeyCode code)
	{
		return glfwGetKey(Application::Get().GetWindow().pd.window, code) == GLFW_PRESS;
	}
	bool Input::IsKeyJustPressed(KeyCode code)
	{
		bool a = 0;
		bool first = 1;
		a = IsKeyPressed(code) && (!(KeyRepeatPoll && LastKeyPoll == code));
		if (first == 1) {
			LastKeyPoll = code;
			first = 0;
		}
		if (IsKeyPressed(code)) {
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
		double xpos, ypos;
		glfwGetCursorPos(Application::Get().GetWindow().pd.window, &xpos, &ypos);
		return xpos;
	}
	int Input::GetMouseY(bool wndcoord)
	{
		double xpos, ypos;
		glfwGetCursorPos(Application::Get().GetWindow().pd.window, &xpos, &ypos);
		return ypos;
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
		
		return 0.0f;
	}
	float Input::GetLeftAnalogY(int ID)
	{
		return 0.0f;
	}
	float Input::GetRightAnalogX(int ID)
	{
		return 0.0f;
	}
	float Input::GetRightAnalogY(int ID)
	{
		return 0.0f;
	}
	bool Input::IsGamepadButtonPressed(int ID, int code)
	{
		return false;
	}
	bool Input::IsGamepadButtonJustPressed(int ID, int code)
	{
		return false;
	}
	void Input::VibrateController(int ID, int left, int right)
	{
		
	}
	float Input::GetLeftTriggerState(int ID)
	{
		return 0.0f;
	}
	float Input::GetRightTriggerState(int ID)
	{
		return 0.0f;
	}
}