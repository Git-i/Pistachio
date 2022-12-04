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
	};

}