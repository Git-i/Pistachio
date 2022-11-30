#include "ptpch.h"
#include "WindowsWindow.h"
#include "Pistachio/Log.h"

#include "Pistachio/Event/ApplicationEvent.h"
#include "Pistachio/Event/KeyEvent.h"
#include "Pistachio/Event/MouseEvent.h"

void OnResize(int width, int height)
{
	WindowData& data = *(WindowData*)GetWindowDataPtr();
	Pistachio::WindowResizeEvent event(width, height);
	data.EventCallback(event);
}
bool first = true;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);

		// All painting occurs here, between BeginPaint and EndPaint.
		FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW));

		EndPaint(hwnd, &ps);
	}
	case WM_SIZE:
	{
		if (first) {
			first = false;
		}
		else {
			UINT width = LOWORD(lParam);
			UINT height = HIWORD(lParam);
			if (!(width == 0 && height == 0))
				OnResize(width, height);
		}
	}
	case WM_CLOSE:
	{
		int id = MessageBox(hwnd, L"Do you want to Exit", L"Exit Application", MB_YESNO);
		if (id == IDYES)
		{
			DestroyWindow(hwnd);
		}
	}
	return 0;

}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

namespace Pistachio {
	Window* Window::Create(const WindowInfo& info)
	{
		return new WindowsWindow(info);
	}
	WindowsWindow::WindowsWindow(const WindowInfo& info)
	{
		Init(info, GetModuleHandle(NULL));
	}
	WindowsWindow::~WindowsWindow()
	{
		Shutdown();
	}
	int WindowsWindow::Init(const WindowInfo& info, HINSTANCE hInstance)
	{
		SetWindowDataPtr(&m_data);
		STARTUPINFO si;
		si.wShowWindow = SW_MAXIMIZE;
		GetStartupInfo(&si);
		m_data.title = info.title;
		m_data.height = info.height;
		m_data.width = info.width;
		m_data.vsync = info.vsync;
		// Register the window class.
		const wchar_t CLASS_NAME[] = L"Sample Window Class";

		WNDCLASS wc = { };

		wc.lpfnWndProc = WindowProc;
		wc.hInstance = hInstance;
		wc.lpszClassName = CLASS_NAME;

		RegisterClass(&wc);

		pd.hwnd = CreateWindowEx(
			0,                              // Optional window styles.
			CLASS_NAME,                     // Window class
			info.title,    // Window text
			WS_OVERLAPPEDWINDOW,            // Window style

			// Size and position
			CW_USEDEFAULT, CW_USEDEFAULT, info.width, info.height,

			NULL,       // Parent window    
			NULL,       // Menu
			hInstance,  // Instance handle
			NULL        // Additional application data
		);

		if (pd.hwnd == NULL)
		{
			return 0;
		}
		ShowWindow(pd.hwnd, 10);
		
		if (AllocConsole() == 0)
		{
			// Handle error here. Use ::GetLastError() to get the error.
		}

		// Redirect CRT standard input, output and error handles to the console window.
		FILE* pNewStdout = nullptr;
		FILE* pNewStderr = nullptr;
		FILE* pNewStdin = nullptr;

		::freopen_s(&pNewStdout, "CONOUT$", "w", stdout);
		::freopen_s(&pNewStderr, "CONOUT$", "w", stderr);
		::freopen_s(&pNewStdin, "CONIN$", "r", stdin);

		// Clear the error state for all of the C++ standard streams. Attempting to accessing the streams before they refer
		// to a valid target causes the stream to enter an error state. Clearing the error state will fix this problem,
		// which seems to occur in newer version of Visual Studio even when the console has not been read from or written
		// to yet.
		std::cout.clear();
		std::cerr.clear();
		std::cin.clear();

		std::wcout.clear();
		std::wcerr.clear();
		std::wcin.clear();
		Pistachio::Log::Init();
		return 0;

	}
	void WindowsWindow::Shutdown()
	{
		
	}
	void WindowsWindow::OnUpdate()
	{
		MSG msg = { };
		while (GetMessage(&msg, NULL, 0, 0) > 0)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	void WindowsWindow::SetVsync(bool enabled)
	{
		m_data.vsync = enabled;
	}
	bool WindowsWindow::IsVsync() const
	{
		return m_data.vsync;
	}
}
