#pragma once

#ifdef PT_PLATFORM_WINDOWS
extern Pistachio::Application* Pistachio::CreateApplication();

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    auto app = Pistachio::CreateApplication();
    app->Run();
    delete app;
    return 0;
}

#endif // PT_PLATFORM_WINDOWS
