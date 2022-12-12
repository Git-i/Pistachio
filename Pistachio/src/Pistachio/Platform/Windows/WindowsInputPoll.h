#pragma once
#include "Pistachio/Core/Input.h"
#include "WindowsInputCallbacks.h"
#include "Pistachio/Core/Application.h"

namespace Pistachio {
	int LastKeyPoll = 0;
	bool KeyRepeatPoll = false;
	class WindowsInput : public Input
	{
	protected:
		bool IsKeyPressedImpl(KeyCode code) override;
		bool IsKeyJustPressedImpl(KeyCode code) override; 
		int GetMouseXImpl(bool wndcoord) override;
		int GetMouseYImpl(bool wndcoord) override;

		// Inherited via Input
		float GetAnalogLeftXImpl(int ID) override;
		float GetAnalogLeftYImpl(int ID) override;
		float GetAnalogRightXImpl(int ID) override;
		float GetAnalogRightYImpl(int ID) override;
		bool IsGamepadButtonPressedImpl(int ID, int code) override;
		bool IsGamepadButtonJustPressedImpl(int ID, int code) override;
		void VibrateControllerImpl(int ID, int left, int right) override;
	};

}