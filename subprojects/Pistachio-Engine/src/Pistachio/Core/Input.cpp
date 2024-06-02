#include "ptpch.h"
#include "Application.h"
#include "Input.h"

namespace Pistachio 
{
    bool  Input::IsKeyPressed(KeyCode code)
    {
        return Application::Get().GetInputHandler().IsKeyPressed(code);
    }
	bool  Input::IsKeyJustPressed(KeyCode code)
    {
        return Application::Get().GetInputHandler().IsKeyJustPressed(code);   
    }
	bool  Input::IsMouseButtonPressed(MouseButton code)
    {
        return Application::Get().GetInputHandler().IsMouseButtonPressed(code);
    }
	bool  Input::IsMouseButtonJustPressed(MouseButton code)
    {
        return Application::Get().GetInputHandler().IsMouseButtonJustPressed(code);
    }
	int   Input::GetMouseX(bool WindowCoordinates)
    {
        return Application::Get().GetInputHandler().GetMouseX(WindowCoordinates);
    }
	int   Input::GetMouseY(bool WindowCoordinates)
    {
        return Application::Get().GetInputHandler().GetMouseY(WindowCoordinates);
    }
	float Input::GetLeftAnalogX(int ID)
    {
        return Application::Get().GetInputHandler().GetLeftAnalogX(ID);
    }
	float Input::GetLeftAnalogY(int ID)
    {
        return Application::Get().GetInputHandler().GetLeftAnalogY(ID);
    }
	float Input::GetRightAnalogX(int ID)
    {
        return Application::Get().GetInputHandler().GetRightAnalogX(ID);
    }
	float Input::GetRightAnalogY(int ID)
    {
        return Application::Get().GetInputHandler().GetRightAnalogY(ID);
    }
	bool  Input::IsGamepadButtonPressed(int ID, int code)
    {
        return Application::Get().GetInputHandler().IsGamepadButtonPressed(ID, code);
    }
	bool  Input::IsGamepadButtonJustPressed(int ID, int code)
    {
        return Application::Get().GetInputHandler().IsGamepadButtonJustPressed(ID, code);
    }
	void  Input::VibrateController(int ID, int left, int right)
    {
        return Application::Get().GetInputHandler().VibrateController(ID, left, right);
    }
	float Input::GetLeftTriggerState(int ID)
    {
        return Application::Get().GetInputHandler().GetLeftTriggerState(ID);
    }
	float Input::GetRightTriggerState(int ID)
    {
        return Application::Get().GetInputHandler().GetLeftTriggerState(ID);
    }
}