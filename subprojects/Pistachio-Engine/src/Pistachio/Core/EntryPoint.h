#pragma once
#include "Pistachio/Renderer/RendererBase.h"
#include "Pistachio/Core/Application.h"
#ifdef PT_PLATFORM_WINDOWS
extern Pistachio::Application* Pistachio::CreateApplication();

#ifndef PT_CUSTOM_ENTRY
int APIENTRY wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    std::wstring line = pCmdLine;
    if (auto pos = line.find(L"attach_vs"); pos != std::string::npos)
    {
        DWORD pid = GetCurrentProcessId();
        std::string cmd = "C:\\WINDOWS\\system32\\vsjitdebugger.exe -p " + std::to_string(pid);
        system(cmd.c_str());
    }

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
#endif // !PT_CUSTOM_ENTRY


#endif // PT_PLATFORM_WINDOWS
