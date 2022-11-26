#include "Application.h"
#include "Pistachio/Event/KeyEvent.h"
#include "Pistachio/Log.h"
namespace Pistachio {
	Application::Application()
	{
	}
	Application::~Application()
	{
	}
	void Application::Run()
	{
		KeyPressedEvent e(5);
		if (e.isInCategory(EventCategoryMouse)) {
			PT_WARN(e);
		}
		while (true);
	}
}