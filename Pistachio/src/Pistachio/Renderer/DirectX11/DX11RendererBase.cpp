#include "ptpch.h"
#include "DX11RendererBase.h"
#include "../../Core/Window.h"
#pragma comment(lib, "dxgi.lib")
namespace Pistachio {

	Error DX11RendererBase::CreateDevice(HWND hWnd, IDXGISwapChain** pSwapChain, ID3D11Device** pd3dDevice, ID3D11DeviceContext** pd3dDeviceContext, ID3D11DepthStencilView** pDSV, ID3D11RenderTargetView** pMainRTV)
	{
		PT_PROFILE_FUNCTION()
		IDXGIAdapter* pAdapter;
		IDXGIFactory* pFactory;
		CreateDXGIFactory(IID_PPV_ARGS(&pFactory));
		pFactory->EnumAdapters(0, &pAdapter);
		DXGI_ADAPTER_DESC desc;
		pAdapter->GetDesc(&desc);
		std::wcout << desc.Description;
		WindowData* data = (WindowData*)(GetWindowDataPtr());
		// Setup swap chain
		DXGI_SWAP_CHAIN_DESC sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.BufferCount = 2;
		sd.BufferDesc.Width = data->width;
		sd.BufferDesc.Height = data->height;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = hWnd;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.Windowed = TRUE;
		sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

		UINT createDeviceFlags = 0;
		#ifdef _DEBUG
			createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
		#endif // _DEBUG

		D3D_FEATURE_LEVEL featureLevel;
		const D3D_FEATURE_LEVEL featureLevelArray[3] = { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
		if (D3D11CreateDeviceAndSwapChain(pAdapter, D3D_DRIVER_TYPE_UNKNOWN, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, pSwapChain, pd3dDevice, &featureLevel, pd3dDeviceContext) != S_OK)
			return 1;
		//(*pd3dDevice)->CreateDeferredContext(0,pd3dDeviceContext); 
		D3D11_DEPTH_STENCIL_DESC dsc = {};
		dsc.DepthEnable = TRUE;
		dsc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		dsc.DepthFunc =  D3D11_COMPARISON_LESS_EQUAL;
		ID3D11DepthStencilState* pDSState;
		(*pd3dDevice)->CreateDepthStencilState(&dsc, &pDSState);
		(*pd3dDeviceContext)->OMSetDepthStencilState(pDSState, 1);

		pDSState->Release();
		return 0;
	}

	void DX11RendererBase::CleanupDevice(IDXGISwapChain* pSwapChain, ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dDeviceContext)
	{
		if (pSwapChain) { while ((pSwapChain)->Release()) {}; (pSwapChain) = NULL; }
		if (pd3dDeviceContext) { while ((pd3dDeviceContext)->Release()) {}; (pd3dDeviceContext) = NULL; }
	}

	void DX11RendererBase::CreateRenderTarget(IDXGISwapChain* pSwapChain, ID3D11Device* pd3dDevice,ID3D11RenderTargetView** pMainRenderTargetView)
	{
		ID3D11Texture2D* pBackBuffer;
		pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
		pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, pMainRenderTargetView);
		pBackBuffer->Release();
	}
	void DX11RendererBase::CleanupRenderTarget(ID3D11RenderTargetView* pMainRenderTargetView, ID3D11DeviceContext* pContext, ID3D11DepthStencilView* pDSV)
	{
		if (pMainRenderTargetView) {
			ID3D11RenderTargetView* nullViews[] = { nullptr };
			pContext->OMSetRenderTargets(1, nullViews, 0);
		    while (pMainRenderTargetView->Release()) {};
			pMainRenderTargetView = NULL; 
			while (pDSV->Release()) {};
			pDSV = NULL;
		}
	}
}
