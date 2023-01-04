#pragma once
#include "Pistachio/Core.h"
#include "Buffer.h"
#include "DirectX11/DX11RendererBase.h"
#include <atlbase.h>
namespace Pistachio {
	enum class CullMode {
		None, Front, Back
	};
	enum class PrimitiveTopology
	{
		TriangleList, LineList, LineStrip, Points, TriangleStrip
	};
	class RendererBase
	{
	public:
		static bool IsDeviceNull;
		static void Shutdown();
		static void ChangeViewport(int width, int height, int x=0, int y=0);
		static void ClearTarget();
		static void CreateTarget();
		static void ClearView();
		static void Resize(int width, int height);
		static void SetPrimitiveTopology(PrimitiveTopology Topology);
		static void SetClearColor(float r, float g, float b, float a);
		static void DrawIndexed(Buffer& buffer);
		static void SetCullMode(CullMode cullmode);
		#ifdef PT_PLATFORM_WINDOWS
			static bool Init(HWND hwnd);
		#endif
		#ifdef PISTACHIO_RENDER_API_DX11
			static inline ID3D11Device*& Getd3dDevice() { return g_pd3dDevice; }
			static inline ID3D11DeviceContext*& Getd3dDeviceContext() { return g_pd3dDeviceContext; }
			static inline IDXGISwapChain*& GetSwapChain() { return g_pSwapChain; }
			static inline ID3D11RenderTargetView*& GetmainRenderTargetView() { return g_mainRenderTargetView; }
			static inline ID3D11DepthStencilView*& GetDepthStencilView(){ return pDSV; }
		#endif
	private:
		#ifdef PISTACHIO_RENDER_API_DX11
			static ID3D11Device* g_pd3dDevice;
			static ID3D11DeviceContext* g_pd3dDeviceContext;
			static IDXGISwapChain* g_pSwapChain;
			static ID3D11RenderTargetView* g_mainRenderTargetView;
			static ID3D11DepthStencilView* pDSV;
			static ID3D11RasterizerState* pRasterizerStateNoCull;
			static ID3D11RasterizerState* pRasterizerStateCWCull;
			static ID3D11RasterizerState* pRasterizerStateCCWCull;
		#endif
		static FLOAT m_ClearColor[4];
	};
}


