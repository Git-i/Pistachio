#pragma once
#include "Pistachio/Core/Error.h"
namespace Pistachio {
	class DX11RendererBase
	{
	public:
		static Error CreateDevice(HWND hWnd, IDXGISwapChain** pSwapChain, ID3D11Device** pd3dDevice, ID3D11DeviceContext** pd3dDeviceContext, ID3D11DepthStencilView** pDSV, ID3D11RenderTargetView** pRenderTargetView);
		static void CleanupDevice(IDXGISwapChain** pSwapChain, ID3D11Device** pd3dDevice, ID3D11DeviceContext** pd3dDeviceContext);
		static void CreateRenderTarget(IDXGISwapChain* pSwapChain, ID3D11Device* pd3dDevice, ID3D11RenderTargetView** pMainRenderTargetView);
		static void CleanupRenderTarget(ID3D11RenderTargetView* pMainRenderTargetView);
	};
}