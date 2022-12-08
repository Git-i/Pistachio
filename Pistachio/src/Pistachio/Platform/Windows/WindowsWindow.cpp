#include "ptpch.h"
#include "WindowsWindow.h"
#include "Pistachio/Core/Log.h"
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

#include "Pistachio/Event/ApplicationEvent.h"
#include "Pistachio/Event/KeyEvent.h"
#include "Pistachio/Event/MouseEvent.h"
#include "Pistachio/Platform/Windows/WindowsInputCallbacks.h"
#include "Pistachio/Renderer/RendererBase.h"
#include "Pistachio/Renderer/Buffer.h"
#include "Pistachio/Renderer/Shader.h"

void* WindowDataPtr = new void*;

void* GetWindowDataPtr()
{
	return WindowDataPtr;
}
void SetWindowDataPtr(void* value)
{
	WindowDataPtr = value;
}



LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
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
		if (!Pistachio::Renderer::IsDeviceNull && wParam != SIZE_MINIMIZED)
		{
			Pistachio::Renderer::Resize((FLOAT)width, (FLOAT)height);
		}
		break;
	}
	case WM_CLOSE:
	{
		int id = MessageBoxW(hwnd, L"Do you want to Exit", L"Exit Application", MB_ICONWARNING | MB_YESNO);
		if (id == IDYES)
		{
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
		if (!ImGui::GetIO().WantCaptureMouse)
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
		ImGuiIO& io = ImGui::GetIO();
		if (!io.WantCaptureMouse)
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
		if (!ImGui::GetIO().WantCaptureMouse)
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
		if (!ImGui::GetIO().WantCaptureMouse)
		Pistachio::OnMouseButtonRelease(0);
		break;
	}
	case WM_RBUTTONUP:
	{
		KeyRepeat = false;
		Pistachio::KeyRepeatPoll = false;
		if (!ImGui::GetIO().WantCaptureMouse)
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
		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DpiEnableScaleViewports)
		{
			//const int dpi = HIWORD(wParam);
			//printf("WM_DPICHANGED to %d (%.0f%%)\n", dpi, (float)dpi / 96.0f * 100.0f);
			const RECT* suggested_rect = (RECT*)lParam;
			::SetWindowPos(hwnd, NULL, suggested_rect->left, suggested_rect->top, suggested_rect->right - suggested_rect->left, suggested_rect->bottom - suggested_rect->top, SWP_NOZORDER | SWP_NOACTIVATE);
		}
		break;
	case WM_MOUSEWHEEL:
	{
		
		float xPos =0;
		float yPos = GET_WHEEL_DELTA_WPARAM(wParam);
		Pistachio::OnMousseScroll(xPos, yPos);
		break;
	}

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
		Init(info, GetModuleHandleA(NULL));
	}
	WindowsWindow::~WindowsWindow()
	{
		Shutdown();
	}
	int WindowsWindow::Init(const WindowInfo& info, HINSTANCE hInstance)
	{
		SetWindowDataPtr(&m_data);
		STARTUPINFOA si;
		GetStartupInfoA(&si);
		si.wShowWindow = SW_SHOWDEFAULT;
		int nCmdShow = si.wShowWindow;
		m_data.title = info.title;
		m_data.height = info.height;
		m_data.width = info.width;
		m_data.vsync = info.vsync;
		// Register the window class.
		const wchar_t CLASS_NAME[] = L"Sample Window Class";

		WNDCLASSEXW wc;

		wc.lpfnWndProc = WindowProc;
		wc.hInstance = hInstance;
		wc.lpszClassName = CLASS_NAME;
		wc.cbSize = sizeof(WNDCLASSEXA);
		wc.style = 0;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = hInstance;
		wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = NULL;
		wc.lpszMenuName = NULL;
		wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

		RegisterClassExW(&wc);
		RECT wr;
		wr.left = 100;
		wr.right = info.width + 100;
		wr.top = 100;
		wr.bottom = 100 + info.height;
		AdjustWindowRectEx(&wr, WS_OVERLAPPEDWINDOW, FALSE, 0);
		pd.hwnd = CreateWindowExW(
			0,
			CLASS_NAME,
			(wchar_t*)info.title,
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT, wr.right-wr.left, wr.bottom-wr.top,
			NULL,       
			NULL,       
			hInstance,  
			NULL        
		);
		if (pd.hwnd == NULL)
		{
			return 0;
		}
		

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
		ShowWindow(pd.hwnd, SW_SHOW);
		Pistachio::Log::Init();
		Renderer::Init(pd.hwnd);

		return 0;

	}
	void WindowsWindow::Shutdown()
	{
		// Cleanup
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();

		Renderer::Shutdown();
		::DestroyWindow(pd.hwnd);
		::UnregisterClassW(L"Sample Window Class", GetModuleHandleA(NULL));
	}
	void WindowsWindow::OnUpdate()
	{
		struct Vertex
		{
			float x, y, r, g, b;
		};
		Vertex vertices[6]
		{
			{ 0.0f,  0.5f, 1.0f, 0,   0.19f,   },
			{ 0.5f, -0.5f, 1.0f, 0,   1.0f, },
			{-0.5f, -0.5f, 1.0f, 0,   1.0f, },
			{-0.3f,  0.3f, 1.0f, 0,   0.19f,   },
			{ 0.3f,  0.3f, 1.0f, 0.0f, 0.19f,   },
			{ 0.0f, -0.8f, 1.0f, 0,   1.0f, }
		};
		const unsigned short indices[]
		{
			0, 1, 2,
			0, 2, 3,
			0, 4, 1,
			2, 1, 5
		};
		VertexBuffer vertexbuffer;
		vertexbuffer.Initialize(vertices, sizeof(vertices), sizeof(Vertex));

		IndexBuffer indexbuffer;
		indexbuffer.Initialize(indices, sizeof(indices), sizeof(unsigned short));

		vertexbuffer.Bind();
		indexbuffer.Bind();

		Shader BasicShader(L"VertexShader.cso", L"PixelShader.cso");
		BasicShader.Bind(ShaderType::Vertex);
		BasicShader.Bind(ShaderType::Pixel);

		BufferLayout layout[] = {
			{"POSITION", BufferLayoutFormat::FLOAT2, 0},
			{"COLOR", BufferLayoutFormat::FLOAT3, 8}
		};
		
		BasicShader.CreateLayout(layout, 2);
		
		//Set Some Necessary Flags-----------------------------------------------------------
		
		//Set the Primitive Topology
		Renderer::g_pd3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//Set Render Target
		Renderer::g_pd3dDeviceContext->OMSetRenderTargets(1, &Renderer::g_mainRenderTargetView, NULL);

		//----------------------------------------------------------------------------------

		//Clear The Back Buffer (This could be done atop the main loop)
		Renderer::ClearView();

		//Draw our Triangles
		Renderer::g_pd3dDeviceContext->DrawIndexed(12, 0, 0);
	}
	void WindowsWindow::EndFrame() const
	{
		Renderer::g_pSwapChain->Present(m_data.vsync, 0);
	}
	void WindowsWindow::SetVsync(bool enabled)
	{
	}
	bool WindowsWindow::IsVsync() const
	{
		return m_data.vsync;
	}
}

