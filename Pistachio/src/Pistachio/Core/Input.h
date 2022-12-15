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
		inline static bool IsKeyPressed(KeyCode code) { return s_Instance->IsKeyPressedImpl(code); }
		inline static bool IsKeyJustPressed(KeyCode code) { return s_Instance->IsKeyJustPressedImpl(code); }
		inline static bool IsMouseButtonPressed(MouseButton code) { return s_Instance->IsKeyPressedImpl(code); }
		inline static bool IsMouseButtonJustPressed(MouseButton code) { return s_Instance->IsKeyJustPressedImpl(code); }
		inline static int GetMouseX(bool WindowCoordinates = true) { return s_Instance->GetMouseXImpl(WindowCoordinates); }
		inline static int GetMouseY(bool WindowCoordinates = true) { return s_Instance->GetMouseYImpl(WindowCoordinates); }
		inline static float GetLeftAnalogX(int ID) { return s_Instance->GetAnalogLeftXImpl(ID); }
		inline static float GetLeftAnalogY(int ID) { return s_Instance->GetAnalogLeftYImpl(ID); }
		inline static float GetRightAnalogX(int ID) { return s_Instance->GetAnalogRightXImpl(ID); }
		inline static float GetRightAnalogY(int ID) { return s_Instance->GetAnalogRightYImpl(ID); }
		inline static bool IsGamepadButtonPressed(int ID, int code) { return s_Instance->IsGamepadButtonPressedImpl(ID, code); }
		inline static bool IsGamepadButtonJustPressed(int ID, int code) { return s_Instance->IsGamepadButtonJustPressedImpl(ID, code); }
		inline static void VibrateController(int ID, int left, int right) { s_Instance->VibrateControllerImpl(ID, left, right); }
		inline static float GetLeftTriggerState(int ID) { return s_Instance->GetLeftTriggerStateImpl(ID); }
		inline static float GetRightTriggerState(int ID) { return s_Instance->GetRightTriggerStateImpl(ID); }
	protected:
		virtual bool IsKeyPressedImpl(KeyCode code) = 0;
		virtual int GetMouseXImpl(bool) = 0;
		virtual int GetMouseYImpl(bool) = 0;
		virtual bool IsKeyJustPressedImpl(KeyCode code) = 0;
		virtual float GetAnalogLeftXImpl(int ID) = 0;
		virtual float GetAnalogLeftYImpl(int ID) = 0;
		virtual float GetAnalogRightXImpl(int ID) = 0;
		virtual float GetAnalogRightYImpl(int ID) = 0;
		virtual bool IsGamepadButtonPressedImpl(int ID, int code) = 0;
		virtual bool IsGamepadButtonJustPressedImpl(int ID, int code) = 0;
		virtual void VibrateControllerImpl(int ID, int left, int right) = 0;
		virtual float GetLeftTriggerStateImpl(int ID) = 0;
		virtual float GetRightTriggerStateImpl(int ID) = 0;
	private:
		static Input* s_Instance;
	};
}
