#pragma once

#include <Pistachio/Application.h>

#ifdef PT_PLATFORM_WINDOWS
extern Pistachio::Application* Pistachio::CreateApplication();

int main(int argc, char** argv)
{
	auto app = Pistachio::CreateApplication();
	app->Run();
	delete app;
}
#endif // PT_PLATFORM_WINDOWS
