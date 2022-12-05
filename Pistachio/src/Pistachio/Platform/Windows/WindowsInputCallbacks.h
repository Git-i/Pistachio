#pragma once
#include "Pistachio/Event/ApplicationEvent.h"
#include "Pistachio/Event/KeyEvent.h"
#include "Pistachio/Event/MouseEvent.h"
#include "Pistachio/Core/Window.h"
extern bool KeyRepeat;
namespace Pistachio {
	extern int LastKey;
	extern bool KeyRepeatPoll;
	void OnMousseScroll(float xamount, float yamount);
	void OnResize(int width, int height);
	void OnKeyDown(int code);
	void OnMouseButtonPress(int button);
	void OnMouseButtonRelease(int button);
	void OnMouseMove(int x, int y);
}