#pragma once

namespace Pistachio {
	class DX11RendererBase
	{
	public:
		static ID3D11RenderTargetView* CreateDevice(HWND hWnd, IDXGISwapChain** pSwapChain, ID3D11Device** pd3dDevice, ID3D11DeviceContext** pd3dDeviceContext, ID3D11DepthStencilView** pDSV);
		static void CleanupDevice(IDXGISwapChain** pSwapChain, ID3D11Device** pd3dDevice, ID3D11DeviceContext** pd3dDeviceContext);
		static void CreateRenderTarget(IDXGISwapChain* pSwapChain, ID3D11Device* pd3dDevice, ID3D11RenderTargetView** pMainRenderTargetView);
		static void CleanupRenderTarget(ID3D11RenderTargetView* pMainRenderTargetView);
	};
}