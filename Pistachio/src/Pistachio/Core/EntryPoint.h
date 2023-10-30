#pragma once
#include "Pistachio/Renderer/RendererBase.h"
#include "Pistachio/Core/Application.h"
#ifdef PT_PLATFORM_WINDOWS
extern Pistachio::Application* Pistachio::CreateApplication();

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    PT_PROFILE_BEGIN_SESSION("Startup", "Pistachio-Startup-Profile.json");
    auto app = Pistachio::CreateApplication();
    PT_PROFILE_END_SESSION();

    PT_PROFILE_BEGIN_SESSION("Runtime", "Pistachio-Runtime-Profile.json");
    app->Run();
    PT_PROFILE_END_SESSION();

    PT_PROFILE_BEGIN_SESSION("Shutdown", "Pistachio-Shutdown-Profile.json");
    delete app;
    PT_PROFILE_END_SESSION();

    return 0;
}

#endif // PT_PLATFORM_WINDOWS
