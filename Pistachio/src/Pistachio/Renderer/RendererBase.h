#pragma once
#include "Pistachio/Core.h"
#include "Buffer.h"
#include "DirectX11/DX11RendererBase.h"
namespace Pistachio {
	enum class CullMode {
		None, Front, Back
	};
	enum class PrimitiveTopology
	{
		TriangleList, LineList, LineStrip, Points, TriangleStrip
	};
	enum class DepthStencilOp {
		Less, Less_Equal
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
		static void DrawIndexed(const Buffer& buffer, unsigned int indexCount=0);
		static void SetCullMode(CullMode cullmode);
		static void EnableShadowMapRasetrizerState();
		static void SetDepthStencilOp(DepthStencilOp op);
		#ifdef PT_PLATFORM_WINDOWS
			static bool Init(HWND hwnd);
		#endif
		#ifdef PISTACHIO_RENDER_API_DX11
			static ID3D11Device* Getd3dDevice() {  return g_pd3dDevice.Get(); }
			static ID3D11DeviceContext* Getd3dDeviceContext() { return g_pd3dDeviceContext.Get(); }
			static IDXGISwapChain* GetSwapChain() { return g_pSwapChain.Get(); }
			static ID3D11RenderTargetView* GetmainRenderTargetView() { return g_mainRenderTargetView.Get(); }
			static ID3D11DepthStencilView* GetDepthStencilView(){ return pDSV.Get(); }
		#endif
	private:
		#ifdef PISTACHIO_RENDER_API_DX11
			static Microsoft::WRL::ComPtr<ID3D11Device> g_pd3dDevice;
			static Microsoft::WRL::ComPtr<ID3D11DeviceContext> g_pd3dDeviceContext;
			static Microsoft::WRL::ComPtr<IDXGISwapChain> g_pSwapChain;
			static Microsoft::WRL::ComPtr<ID3D11RenderTargetView> g_mainRenderTargetView;
			static Microsoft::WRL::ComPtr<ID3D11DepthStencilView> pDSV;
			static Microsoft::WRL::ComPtr<ID3D11RasterizerState> pRasterizerStateNoCull;
			static Microsoft::WRL::ComPtr<ID3D11RasterizerState> pShadowMapRasterizerState;
			static Microsoft::WRL::ComPtr<ID3D11RasterizerState> pRasterizerStateCWCull;
			static Microsoft::WRL::ComPtr<ID3D11RasterizerState> pRasterizerStateCCWCull;
			static Microsoft::WRL::ComPtr<ID3D11DepthStencilState> pDSStateLess;
			static Microsoft::WRL::ComPtr<ID3D11DepthStencilState> pDSStateLessEqual;
		#endif
		static FLOAT m_ClearColor[4];
	};
}


