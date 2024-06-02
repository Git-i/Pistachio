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
		static int GetMouseX(bool WindowCoordinates = false);
		static int GetMouseY(bool WindowCoordinates = false);
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
	class PISTACHIO_API InputHandler
	{
	public:
		virtual bool IsKeyPressed(KeyCode code) = 0;
		virtual bool IsKeyJustPressed(KeyCode code) = 0;
		virtual bool IsMouseButtonPressed(MouseButton code) = 0;
		virtual bool IsMouseButtonJustPressed(MouseButton code) = 0;
		virtual int GetMouseX(bool WindowCoordinates = false) = 0;
		virtual int GetMouseY(bool WindowCoordinates = false) = 0;
		virtual float GetLeftAnalogX(int ID) = 0;
		virtual float GetLeftAnalogY(int ID) = 0;
		virtual float GetRightAnalogX(int ID) = 0;
		virtual float GetRightAnalogY(int ID) = 0;
		virtual bool IsGamepadButtonPressed(int ID, int code) = 0;
		virtual bool IsGamepadButtonJustPressed(int ID, int code) = 0;
		virtual void VibrateController(int ID, int left, int right) = 0;
		virtual float GetLeftTriggerState(int ID) = 0;
		virtual float GetRightTriggerState(int ID) = 0;
	};
}
