#pragma once

#ifdef PT_PLATFORM_WINDOWS
extern Pistachio::Application* Pistachio::CreateApplication();

int main(int argc, char** argv)
{
	Pistachio::Log::Init();
	auto app = Pistachio::CreateApplication();
	app->Run();
	delete app;
}
#endif // PT_PLATFORM_WINDOWS
