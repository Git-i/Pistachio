#include "ptpch.h"
#include "WindowsWindow.h"
#include "Pistachio/Core/Log.h"
#ifdef IMGUI
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#endif

#include "Pistachio/Event/ApplicationEvent.h"
#include "Pistachio/Event/KeyEvent.h"
#include "Pistachio/Event/MouseEvent.h"
#include "Pistachio/Platform/Windows/WindowsInputCallbacks.h"

#pragma comment(lib, "shell32.lib")
void* WindowDataPtr = new void*;

void* GetWindowDataPtr()
{
	return WindowDataPtr;
}
void SetWindowDataPtr(void* value)
{
	WindowDataPtr = value;
}



extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))
		return true;
	switch (uMsg)
	{
		case WM_MOUSEMOVE:
		{
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);
			Pistachio::OnMouseMove(x, y);
			break;
		}
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
			break;
		case WM_SIZE:
		{
			UINT width = LOWORD(lParam);
			UINT height = HIWORD(lParam);
			Pistachio::OnResize(width, height);
			break;
		}
		case WM_CLOSE:
		{
			int id = MessageBoxW(hwnd, L"Do you want to Exit", L"Exit Application", MB_ICONWARNING | MB_YESNO);
			if (id == IDYES)
			{
#ifdef IMGUI


				ImGui_ImplDX11_Shutdown();
				ImGui_ImplWin32_Shutdown();
				ImGui::DestroyContext();
#endif // IMGUI
				DestroyWindow(hwnd);
			}
			else
			{
				return 0;
			}
			break;
		}
		case WM_KEYDOWN:
		{
			 Pistachio::OnKeyDown((int)wParam);
			if ((int)wParam == Pistachio::LastKey)
				KeyRepeat = true;
			else
				KeyRepeat = false;
			Pistachio::LastKey = (int)wParam;
			break;
		}
		case WM_KEYUP:
		{
			KeyRepeat = false;
			Pistachio::KeyRepeatPoll = false;
			break;
		}
		case WM_LBUTTONDOWN:
		{
			Pistachio::OnMouseButtonPress(0);
			if (0x01 == Pistachio::LastKey)
				KeyRepeat = true;
			else
				KeyRepeat = false;
			Pistachio::LastKey = 0x01;
			break;
		}
		case WM_RBUTTONDOWN:
		{
			Pistachio::OnMouseButtonPress(1);
			if (0x02 == Pistachio::LastKey)
				KeyRepeat = true;
			else
				KeyRepeat = false;
			Pistachio::LastKey = 0x02;
			break;

		}
		case WM_MBUTTONDOWN:
		{
			Pistachio::OnMouseButtonPress(2);
			if (0x04 == Pistachio::LastKey)
				KeyRepeat = true;
			else
				KeyRepeat = false;
			Pistachio::LastKey = 0x04;
			break;
		}
		case WM_LBUTTONUP:
		{
			KeyRepeat = false;
			Pistachio::KeyRepeatPoll = false;
			Pistachio::OnMouseButtonRelease(0);
			break;
		}
		case WM_RBUTTONUP:
		{
			KeyRepeat = false;
			Pistachio::KeyRepeatPoll = false;
			Pistachio::OnMouseButtonRelease(1);
			break;

		}
		case WM_MBUTTONUP:
		{
			KeyRepeat = false;
			Pistachio::KeyRepeatPoll = false;
			Pistachio::OnMouseButtonRelease(2);
			break;
		}
		case WM_DPICHANGED:
#ifdef IMGUI


			if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DpiEnableScaleViewports)
			{
				//const int dpi = HIWORD(wParam);
				//printf("WM_DPICHANGED to %d (%.0f%%)\n", dpi, (float)dpi / 96.0f * 100.0f);
				const RECT* suggested_rect = (RECT*)lParam;
				::SetWindowPos(hwnd, NULL, suggested_rect->left, suggested_rect->top, suggested_rect->right - suggested_rect->left, suggested_rect->bottom - suggested_rect->top, SWP_NOZORDER | SWP_NOACTIVATE);
			}
#endif // IMGUI
			PT_CORE_INFO("DPI CHANGED");
			break;
		case WM_MOUSEWHEEL:
		{
		
			float xPos = ((short)LOWORD(wParam));
			float yPos = GET_WHEEL_DELTA_WPARAM(wParam);
			Pistachio::OnMouseScroll(xPos, yPos);
			break;
		}
		case WM_SETFOCUS:
		{
			//if (!Pistachio::RendererBase::IsDeviceNull)
				//Pistachio::RendererBase::GetSwapChain()->SetFullscreenState(TRUE, NULL);
			break;
		}
		case WM_DROPFILES:
		{
			char ch[256];
			DragQueryFileA((HDROP)wParam, 0, ch, 256);
			WindowData& data = *(WindowData*)GetWindowDataPtr();
			Pistachio::FileDropEvent event(ch);
			if (data.EventCallback != NULL) {
				data.EventCallback(event);
			}
			break;
		}
		printf("l");
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}



namespace Pistachio {
	
	Window* Window::Create(const WindowInfo& info, bool m_headless)
	{
		return new WindowsWindow(info, m_headless);
	}
	WindowsWindow::WindowsWindow(const WindowInfo& info, bool headless)
	{
		Init(info, GetModuleHandleA(NULL), headless);
	}
	WindowsWindow::~WindowsWindow()
	{
		Shutdown();
	}
	int WindowsWindow::Init(const WindowInfo& info, HINSTANCE hInstance, bool headless)
	{
		PT_PROFILE_FUNCTION();
		//SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
		SetWindowDataPtr(&m_data);
		m_data.title = info.title;
		m_data.height = info.height;
		m_data.width = info.width;
		m_data.vsync = info.vsync;
		// Register the window class.
		const wchar_t CLASS_NAME[] = L"Pitsachio Window Class";

		WNDCLASSEXW wc;
		wc.lpfnWndProc = WindowProc;
		wc.hInstance = hInstance;
		wc.lpszClassName = CLASS_NAME;
		wc.cbSize = sizeof(WNDCLASSEXW);
		wc.style = 64;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = hInstance;
		wc.hIcon = (HICON)(LoadImageA(hInstance, MAKEINTRESOURCEA(102), IMAGE_ICON, 32, 32, 0));
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = NULL;
		wc.lpszMenuName = NULL;
		wc.hIconSm = (HICON)(LoadImageA(hInstance,MAKEINTRESOURCEA(102),IMAGE_ICON,16,16,0));
		RegisterClassExW(&wc);
		RECT wr;
		wr.left = 100;
		wr.right = info.width + 100;
		wr.top = 100;
		wr.bottom = 100 + info.height;
		AdjustWindowRectExForDpi(&wr, WS_OVERLAPPEDWINDOW, FALSE, 0, 250);
		pd.hwnd = CreateWindowExW(
			0,
			CLASS_NAME,
			(wchar_t*)info.title,
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT, (UINT)((wr.right - wr.left)/* * (float)GetDpiForSystem() / 96.f*/), (UINT)((wr.bottom - wr.top) /** (float)GetDpiForSystem() / 96.f*/),
			NULL,       
			NULL,
			hInstance,  
			NULL        
		);
		if (pd.hwnd == NULL)
		{
			return 0;
		}
		DragAcceptFiles(pd.hwnd, TRUE);
#if _DEBUG
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

		SetConsoleTitleW(L"Pistachio Application Debug Console");
#endif
		m_data.dpiscale = 1;// (float)GetDpiForWindow(pd.hwnd) / 96.f;
		ShowWindow(pd.hwnd, SW_SHOWDEFAULT);
		if (headless) ShowWindow(pd.hwnd, SW_HIDE);
		return 0;

	}
	void WindowsWindow::Shutdown()
	{
		::DestroyWindow(pd.hwnd);
		::UnregisterClassW(L"Sample Window Class", GetModuleHandleA(NULL));
	}
	void WindowsWindow::OnUpdate(float delta)
	{
		#ifdef _DEBUG
			char buf[10];
			int a = int(1/delta);
			_itoa_s(a, buf, 10);
			std::string title = std::string("FPS: ") + buf;
			SetWindowTextW(pd.hwnd, (wchar_t*)title.c_str());
		#endif // _DEBUG
	}
	void WindowsWindow::SetVsync(unsigned int enabled)
	{
		m_data.vsync = enabled;
	}
	unsigned int WindowsWindow::IsVsync() const
	{
		return m_data.vsync;
	}
}

