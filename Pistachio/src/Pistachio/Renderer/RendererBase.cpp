#include "ptpch.h"
#include "RendererBase.h"
#include "Pistachio/Core/Log.h"
#include "Pistachio/Core/Window.h"

ID3D11Device* Pistachio::Renderer::g_pd3dDevice = NULL;
ID3D11DeviceContext* Pistachio::Renderer::g_pd3dDeviceContext = NULL;
IDXGISwapChain* Pistachio::Renderer::g_pSwapChain = NULL;
ID3D11RenderTargetView* Pistachio::Renderer::g_mainRenderTargetView = NULL;
bool Pistachio::Renderer::IsDeviceNull = true;
namespace Pistachio {
	//General Pistachio Renderer API calls
	void Renderer::Shutdown()
	{
	#ifdef PISTACHIO_RENDER_API_DX11
		CleanupDeviceD3D11();
	#endif
	}
	bool Renderer::Init(HWND hwnd)
	{
	#ifdef PISTACHIO_RENDER_API_DX11
		if (!CreateDeviceD3D11(hwnd))
		{
			CleanupDeviceD3D11();
			return 1;
		}
		PT_CORE_INFO("Renderer Initialized with API: DirectX 11");
		Renderer::Resize((FLOAT)((WindowData*)GetWindowDataPtr())->width, (FLOAT)((WindowData*)GetWindowDataPtr())->height);
		IsDeviceNull = false;
		return 0;
	#endif 
	}
	void Renderer::ClearTarget()
	{
	#ifdef PISTACHIO_RENDER_API_DX11
		CleanupRenderTargetD3D11();
	#endif
	}
	void Renderer::CreateTarget()
	{
	#ifdef PISTACHIO_RENDER_API_DX11
		CreateRenderTargetD3D11();
	#endif
	}
	void Renderer::ClearView()
	{
#ifdef PISTACHIO_RENDER_API_DX11
		FLOAT backgroundColor[4] = { 0.2f, 0.2f, 0.19f, 1.0f };
		Renderer::g_pd3dDeviceContext->ClearRenderTargetView(Renderer::g_mainRenderTargetView, backgroundColor);
#endif // PISTACHIO_RENDER_API_DX11

	}

	void Renderer::Resize(float width, float height)
	{
		ClearTarget();
		g_pSwapChain->ResizeBuffers(0, (UINT)width, (UINT)height, DXGI_FORMAT_UNKNOWN, 0);
		CreateTarget();
		D3D11_VIEWPORT vp;
		vp.Width = width;
		vp.Height = height;
		vp.MinDepth = 0;
		vp.MaxDepth = 1;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		Renderer::g_pd3dDeviceContext->RSSetViewports(1, &vp);
	}

	//D3D11 Renderer Functions
	bool Renderer::CreateDeviceD3D11(HWND hWnd)
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

		CreateRenderTargetD3D11();
		return true;
	}

	void Renderer::CleanupDeviceD3D11()
	{
		CleanupRenderTargetD3D11();
		if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = NULL; }
		if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL; }
		if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
	}

	void Renderer::CreateRenderTargetD3D11()
	{
		ID3D11Texture2D* pBackBuffer;
		g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
		g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
		pBackBuffer->Release();
	}

	void Renderer::CleanupRenderTargetD3D11()
	{
		if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = NULL; }
	}
}
