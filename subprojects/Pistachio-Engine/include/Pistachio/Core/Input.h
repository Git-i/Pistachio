#pragma once
#include "Pistachio/Core.h"

enum MouseButton
{
	PT_MOUSE_LEFT = 0x01,
	PT_MOUSE_RIGHT = 0x02,
	PT_MOUSE_MIDDLE = 0x04
};

namespace Pistachio {
	class PISTACHIO_API Input
	{
	public:
		static bool IsKeyPressed(KeyCode code);
		static bool IsKeyJustPressed(KeyCode code);
		static bool IsMouseButtonPressed(MouseButton code);
		static bool IsMouseButtonJustPressed(MouseButton code);
		static int GetMouseX(bool WindowCoordinates = true);
		static int GetMouseY(bool WindowCoordinates = true);
		static float GetLeftAnalogX(int ID);
		static float GetLeftAnalogY(int ID);
		static float GetRightAnalogX(int ID);
		static float GetRightAnalogY(int ID);
		static bool IsGamepadButtonPressed(int ID, int code);
		static bool IsGamepadButtonJustPressed(int ID, int code);
		static void VibrateController(int ID, int left, int right);
		static float GetLeftTriggerState(int ID);
		static float GetRightTriggerState(int ID);
	};
}
