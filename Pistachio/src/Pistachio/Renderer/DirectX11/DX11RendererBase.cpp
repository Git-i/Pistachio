#include "ptpch.h"
#include "DX11RendererBase.h"
namespace Pistachio {
	ID3D11RenderTargetView* DX11RendererBase::CreateDevice(HWND hWnd, IDXGISwapChain** pSwapChain, ID3D11Device** pd3dDevice, ID3D11DeviceContext** pd3dDeviceContext)
	{
		ID3D11RenderTargetView*  result = nullptr;
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
		if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, pSwapChain, pd3dDevice, &featureLevel, pd3dDeviceContext) != S_OK)
			return nullptr;

		CreateRenderTarget(*pSwapChain, *pd3dDevice, &result);
		return result;
	}

	void DX11RendererBase::CleanupDevice(IDXGISwapChain** pSwapChain, ID3D11Device** pd3dDevice, ID3D11DeviceContext** pd3dDeviceContext)
	{
		if (*pSwapChain) { (*pSwapChain)->Release(); (*pSwapChain) = NULL; }
		if (*pd3dDeviceContext) { (*pd3dDeviceContext)->Release(); (*pd3dDeviceContext) = NULL; }
		if (*pd3dDevice) { (*pd3dDevice)->Release(); (*pd3dDevice) = NULL; }
	}

	void DX11RendererBase::CreateRenderTarget(IDXGISwapChain* pSwapChain, ID3D11Device* pd3dDevice,ID3D11RenderTargetView** pMainRenderTargetView)
	{
		ID3D11Texture2D* pBackBuffer;
		pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
		pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, pMainRenderTargetView);
		pBackBuffer->Release();
	}
	void DX11RendererBase::CleanupRenderTarget(ID3D11RenderTargetView* pMainRenderTargetView)
	{
		if (pMainRenderTargetView) { pMainRenderTargetView->Release(); pMainRenderTargetView = NULL; }
	}
}
