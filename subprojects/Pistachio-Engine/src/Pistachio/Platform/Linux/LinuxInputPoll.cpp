#include "ptpch.h"
#include "LinuxInputHandler.h"
#include "Pistachio/Core/Application.h"
#include "Pistachio/Core/KeyCodes.h"
namespace Pistachio {
	InputHandler* CreateDefaultInputHandler()
	{
		return new LinuxInputHandler;
	}
	bool KeyRepeatPoll;
	int LastKeyPoll;
	bool LinuxInputHandler::IsKeyPressed(KeyCode code)
	{
		if(code >= PT_MOUSE_BUTTON_1 && code <= PT_MOUSE_BUTTON_8) return glfwGetMouseButton(Application::Get().GetWindow().pd.window, code)==GLFW_PRESS;
		return glfwGetKey(Application::Get().GetWindow().pd.window, code) == GLFW_PRESS;
	}
	bool LinuxInputHandler::IsKeyJustPressed(KeyCode code)
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
	int LinuxInputHandler::GetMouseX(bool wndcoord)
	{
		double xpos, ypos;
		glfwGetCursorPos(Application::Get().GetWindow().pd.window, &xpos, &ypos);
		return xpos;
	}
	int LinuxInputHandler::GetMouseY(bool wndcoord)
	{
		double xpos, ypos;
		glfwGetCursorPos(Application::Get().GetWindow().pd.window, &xpos, &ypos);
		return ypos;
	}

	bool LinuxInputHandler::IsMouseButtonPressed(MouseButton button)
	{
		return LinuxInputHandler::IsKeyPressed(button);
	}
	bool LinuxInputHandler::IsMouseButtonJustPressed(MouseButton button)
	{
		return LinuxInputHandler::IsKeyJustPressed(button);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////
	///Gamepad/////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////

	float LinuxInputHandler::GetLeftAnalogX(int ID)
	{
		
		return 0.0f;
	}
	float LinuxInputHandler::GetLeftAnalogY(int ID)
	{
		return 0.0f;
	}
	float LinuxInputHandler::GetRightAnalogX(int ID)
	{
		return 0.0f;
	}
	float LinuxInputHandler::GetRightAnalogY(int ID)
	{
		return 0.0f;
	}
	bool LinuxInputHandler::IsGamepadButtonPressed(int ID, int code)
	{
		return false;
	}
	bool LinuxInputHandler::IsGamepadButtonJustPressed(int ID, int code)
	{
		return false;
	}
	void LinuxInputHandler::VibrateController(int ID, int left, int right)
	{
		
	}
	float LinuxInputHandler::GetLeftTriggerState(int ID)
	{
		return 0.0f;
	}
	float LinuxInputHandler::GetRightTriggerState(int ID)
	{
		return 0.0f;
	}
}