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

ID3D11Device* g_pd3dDevice = NULL;
ID3D11DeviceContext* g_pd3dDeviceContext = NULL;
IDXGISwapChain* g_pSwapChain = NULL;
ID3D11RenderTargetView* g_mainRenderTargetView = NULL;
void* WindowDataPtr = new void*;

void* GetWindowDataPtr()
{
	return WindowDataPtr;
}
void SetWindowDataPtr(void* value)
{
	WindowDataPtr = value;
}
// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();

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
		if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
		{
			CleanupRenderTarget();
			g_pSwapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
			CreateRenderTarget();
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
		GET_WHEEL_DELTA_WPARAM(wParam);
		float xPos =0;
		float yPos =0;
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
		if (!CreateDeviceD3D(pd.hwnd))
		{
			CleanupDeviceD3D();
			::UnregisterClassW(wc.lpszClassName, wc.hInstance);
			return 1;
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
#endif
		ShowWindow(pd.hwnd, SW_SHOW);
		Pistachio::Log::Init();

		return 0;

	}
	void WindowsWindow::Shutdown()
	{
		// Cleanup
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();

		CleanupDeviceD3D();
		::DestroyWindow(pd.hwnd);
		::UnregisterClassA("Sample Window Class", GetModuleHandleA(NULL));
	}
	void WindowsWindow::OnUpdate()
	{
		struct Vertex
		{
			float x, y;
		};
		Vertex vertices[3]
		{
			{0.0f, 0.5f},
			{0.5f, -0.5},
			{-0.5, -0.5}
		};
		ID3D11Buffer* pVertexBuffer = NULL;
		D3D11_BUFFER_DESC bd = {};
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;
		bd.ByteWidth = sizeof(vertices);
		bd.StructureByteStride = sizeof(Vertex);
		D3D11_SUBRESOURCE_DATA sd = {};
		sd.pSysMem = vertices;
		g_pd3dDevice->CreateBuffer(&bd, &sd, &pVertexBuffer);
		UINT stride = sizeof(Vertex);
		UINT offset = 0;

		ID3D11InputLayout* pLayout = NULL;
		const D3D11_INPUT_ELEMENT_DESC ied[]
		{
			{"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0}
		};
		g_pd3dDeviceContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &stride, &offset);
		ID3D11VertexShader* pVertexShader = NULL;
		ID3DBlob* pBlob = NULL;
		ID3D11PixelShader* pPixelShader = NULL;
		D3DReadFileToBlob(L"PixelShader.cso", &pBlob);
		g_pd3dDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pPixelShader);
		g_pd3dDeviceContext->PSSetShader(pPixelShader, nullptr, 0);

		pBlob = NULL;
		D3DReadFileToBlob(L"VertexShader.cso", &pBlob);
		g_pd3dDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pVertexShader);
		g_pd3dDeviceContext->VSSetShader(pVertexShader, nullptr, 0);

		g_pd3dDevice->CreateInputLayout(ied, 1, pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &pLayout);

		g_pd3dDeviceContext->IASetInputLayout(pLayout);
		g_pd3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		D3D11_VIEWPORT vp;
		vp.Width = (FLOAT)((WindowData*)GetWindowDataPtr())->width;
		vp.Height = (FLOAT)((WindowData*)GetWindowDataPtr())->height;
		vp.MinDepth = 0;
		vp.MaxDepth = 1;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		g_pd3dDeviceContext->RSSetViewports(1, &vp);

		g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
		FLOAT backgroundColor[4] = { 0.2f, 0.2f, 0.19f, 1.0f };
		g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, backgroundColor);
		g_pd3dDeviceContext->Draw(3, 0);
	}
	void WindowsWindow::EndFrame() const
	{
		g_pSwapChain->Present(m_data.vsync, 0);
	}
	void WindowsWindow::SetVsync(bool enabled)
	{
	}
	bool WindowsWindow::IsVsync() const
	{
		return m_data.vsync;
	}
}
bool CreateDeviceD3D(HWND hWnd)
{
	// Setup swap chain
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 2;
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	UINT createDeviceFlags = 0;
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	D3D_FEATURE_LEVEL featureLevel;
	const D3D_FEATURE_LEVEL featureLevelArray[3] = { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
	if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
		return false;

	CreateRenderTarget();
	return true;
}

void CleanupDeviceD3D()
{
	CleanupRenderTarget();
	if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = NULL; }
	if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL; }
	if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
}

void CreateRenderTarget()
{
	ID3D11Texture2D* pBackBuffer;
	g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
	g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
	pBackBuffer->Release();
}

void CleanupRenderTarget()
{
	if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = NULL; }
}
