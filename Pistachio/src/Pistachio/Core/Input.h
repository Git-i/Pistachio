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
	protected:
		virtual bool IsKeyPressedImpl(KeyCode code) = 0;
		virtual int GetMouseXImpl(bool) = 0;
		virtual int GetMouseYImpl(bool) = 0;
		virtual bool IsKeyJustPressedImpl(KeyCode code) = 0;
	private:
		static Input* s_Instance;
	};
}
