#pragma once
#include "Pistachio/Core.h"
namespace Pistachio {
	class Renderer
	{
	public:
#ifdef PISTACHIO_RENDER_API_DX11
		static ID3D11Device* g_pd3dDevice;
		static ID3D11DeviceContext* g_pd3dDeviceContext;
		static IDXGISwapChain* g_pSwapChain;
		static ID3D11RenderTargetView* g_mainRenderTargetView;
#endif
		static bool IsDeviceNull;
		static void Shutdown();
		static void ClearTarget();
		static void CreateTarget();
		static void ClearView();
		static void Resize(float width, float height);
#ifdef PT_PLATFORM_WINDOWS
		static bool Init(HWND hwnd);
#endif
	private:
		static bool CreateDeviceD3D11(HWND hWnd);
		static void CleanupDeviceD3D11();
		static void CreateRenderTargetD3D11();
		static void CleanupRenderTargetD3D11();
	};
}


