#include "ptpch.h"
#include "RendererBase.h"
#include "Pistachio/Core/Log.h"
#include "Pistachio/Core/Window.h"
#include "Pistachio/Core/Error.h"
Microsoft::WRL::ComPtr<ID3D11RasterizerState> Pistachio::RendererBase::pRasterizerStateNoCull = NULL;
Microsoft::WRL::ComPtr<ID3D11RasterizerState> Pistachio::RendererBase::pRasterizerStateCWCull = NULL;
Microsoft::WRL::ComPtr<ID3D11RasterizerState> Pistachio::RendererBase::pRasterizerStateCCWCull = NULL;
Microsoft::WRL::ComPtr<ID3D11RasterizerState> Pistachio::RendererBase::pShadowMapRasterizerState = NULL;
Microsoft::WRL::ComPtr<ID3D11DepthStencilState> Pistachio::RendererBase::pDSStateLess = NULL;
Microsoft::WRL::ComPtr<ID3D11DepthStencilState> Pistachio::RendererBase::pDSStateLessEqual = NULL;
Microsoft::WRL::ComPtr<ID3D11Device> Pistachio::RendererBase::g_pd3dDevice = NULL;
Microsoft::WRL::ComPtr<ID3D11DeviceContext> Pistachio::RendererBase::g_pd3dDeviceContext = NULL;
Microsoft::WRL::ComPtr<IDXGISwapChain> Pistachio::RendererBase::g_pSwapChain = NULL;
Microsoft::WRL::ComPtr<ID3D11RenderTargetView> Pistachio::RendererBase::g_mainRenderTargetView = NULL;
Microsoft::WRL::ComPtr<ID3D11DepthStencilView> Pistachio::RendererBase::pDSV = NULL;
bool Pistachio::RendererBase::IsDeviceNull = true;
FLOAT Pistachio::RendererBase::m_ClearColor[4] = {0};
namespace Pistachio {
	//General Pistachio RendererBase API calls
	static D3D11_PRIMITIVE_TOPOLOGY DX11Topology(PrimitiveTopology Topology)
	{
		switch (Topology)
		{
		case Pistachio::PrimitiveTopology::TriangleList: return D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		case Pistachio::PrimitiveTopology::LineList: return D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
		case Pistachio::PrimitiveTopology::LineStrip: return D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
		case Pistachio::PrimitiveTopology::Points: return D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
		case Pistachio::PrimitiveTopology::TriangleStrip: return D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
		default:
			return D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
			break;
		}
	}
	void RendererBase::Shutdown()
	{
	}
	bool RendererBase::Init(HWND hwnd)
	{
		PT_PROFILE_FUNCTION();
	#ifdef PISTACHIO_RENDER_API_DX11
		Error::LogErrorToConsole(DX11RendererBase::CreateDevice(hwnd, &g_pSwapChain, &g_pd3dDevice, &g_pd3dDeviceContext, &pDSV, &g_mainRenderTargetView));
		PT_CORE_INFO("RendererBase Initialized with API: DirectX 11");
		RendererBase::Resize(((WindowData*)GetWindowDataPtr())->width, ((WindowData*)GetWindowDataPtr())->height);
		IsDeviceNull = false;
		g_pd3dDeviceContext->IASetPrimitiveTopology(DX11Topology(PrimitiveTopology::TriangleList));
		D3D11_RASTERIZER_DESC desc = {};
		D3D11_DEPTH_STENCIL_DESC dsc = {};
		dsc.DepthEnable = TRUE;
		dsc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		dsc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
		g_pd3dDevice->CreateDepthStencilState(&dsc, &pDSStateLessEqual);
		dsc.DepthFunc = D3D11_COMPARISON_LESS;
		g_pd3dDevice->CreateDepthStencilState(&dsc, &pDSStateLess);
		desc.FillMode = D3D11_FILL_SOLID;
		desc.CullMode = D3D11_CULL_NONE;
		g_pd3dDevice->CreateRasterizerState(&desc, &pRasterizerStateNoCull);
		desc.CullMode = D3D11_CULL_BACK;
		g_pd3dDevice->CreateRasterizerState(&desc, &pRasterizerStateCWCull);
		desc.CullMode = D3D11_CULL_FRONT;
		g_pd3dDevice->CreateRasterizerState(&desc, &pRasterizerStateCCWCull);
		desc.CullMode = D3D11_CULL_FRONT;
		//desc.DepthBias = 100000;
		//desc.DepthBiasClamp = 0.0f;
		//desc.SlopeScaledDepthBias = 1.0f;
		g_pd3dDevice->CreateRasterizerState(&desc, &pShadowMapRasterizerState);
		return 0;
	#endif 
	}
	void RendererBase::ClearTarget()
	{
		#ifdef PISTACHIO_RENDER_API_DX11
			DX11RendererBase::CleanupRenderTarget(g_mainRenderTargetView.Get(), g_pd3dDeviceContext.Get(), pDSV.Get());
		#endif
	}
	void RendererBase::ChangeViewport(int width, int height, int x, int y)
	{
		D3D11_VIEWPORT vp;
		vp.Width = width;
		vp.Height = height;
		vp.MinDepth = 0;
		vp.MaxDepth = 1;
		vp.TopLeftX = x;
		vp.TopLeftY = y;
		RendererBase::g_pd3dDeviceContext->RSSetViewports(1, &vp);
	}
	void RendererBase::CreateTarget()
	{
		#ifdef PISTACHIO_RENDER_API_DX11
		ID3D11Texture2D* pDepthStencil;
		D3D11_TEXTURE2D_DESC depthTexDesc = {};
		WindowData* data = (WindowData*)(GetWindowDataPtr());
		depthTexDesc.Width = data->width;
		depthTexDesc.Height = data->height;
		depthTexDesc.MipLevels = 1;
		depthTexDesc.ArraySize = 1;
		depthTexDesc.Format = DXGI_FORMAT_D32_FLOAT;
		depthTexDesc.SampleDesc.Count = 1;
		depthTexDesc.SampleDesc.Quality = 0;
		depthTexDesc.Usage = D3D11_USAGE_DEFAULT;
		depthTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		(g_pd3dDevice)->CreateTexture2D(&depthTexDesc, nullptr, &pDepthStencil);
		D3D11_DEPTH_STENCIL_VIEW_DESC dsv = {};
		dsv.Format = DXGI_FORMAT_D32_FLOAT;
		dsv.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		dsv.Texture2D.MipSlice = 0;
		(g_pd3dDevice)->CreateDepthStencilView(pDepthStencil, &dsv, pDSV.ReleaseAndGetAddressOf());
		DX11RendererBase::CreateRenderTarget(g_pSwapChain.Get(), g_pd3dDevice.Get(), g_mainRenderTargetView.ReleaseAndGetAddressOf());
		pDepthStencil->Release();
		#endif
	}
	void RendererBase::ClearView()
	{
		#ifdef PISTACHIO_RENDER_API_DX11
			g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView.Get(), m_ClearColor);
			g_pd3dDeviceContext->ClearDepthStencilView(pDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
		#endif // PISTACHIO_RENDER_API_DX11
	}

	void RendererBase::Resize(int width, int height)
	{
		PT_PROFILE_FUNCTION()
		ID3D11RenderTargetView* nullViews[] = { nullptr };
		g_pd3dDeviceContext->OMSetRenderTargets(1, nullViews, nullptr);
		pDSV = nullptr;
		g_mainRenderTargetView = nullptr;
		g_pSwapChain->ResizeBuffers(0, (UINT)width, (UINT)height, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING);
		CreateTarget();
		ChangeViewport(width, height);
	}

	void RendererBase::SetPrimitiveTopology(PrimitiveTopology Topology)
	{
		g_pd3dDeviceContext->IASetPrimitiveTopology(DX11Topology(Topology));
	}

	void RendererBase::SetClearColor(float r, float g, float b, float a)
	{
		m_ClearColor[0] = r;
		m_ClearColor[1] = g;
		m_ClearColor[2] = b;
		m_ClearColor[3] = a;
	}

	void RendererBase::DrawIndexed(const Buffer& buffer, unsigned int indexCount)
	{
		PT_PROFILE_FUNCTION()
		unsigned int count = indexCount ? indexCount : buffer.ib->GetCount();
		buffer.Bind();
		g_pd3dDeviceContext->DrawIndexed(count, 0, 0);
	}

	void RendererBase::SetCullMode(CullMode cullmode)
	{
		switch (cullmode)
		{
		case Pistachio::CullMode::None: g_pd3dDeviceContext->RSSetState(pRasterizerStateNoCull.Get());
			break;
		case Pistachio::CullMode::Front: g_pd3dDeviceContext->RSSetState(pRasterizerStateCCWCull.Get());
			break;
		case Pistachio::CullMode::Back: g_pd3dDeviceContext->RSSetState(pRasterizerStateCWCull.Get());
			break;
		default:
			break;
		}
	}
	void RendererBase::EnableShadowMapRasetrizerState()
	{
		g_pd3dDeviceContext->RSSetState(pShadowMapRasterizerState.Get());
	}
	void RendererBase::SetDepthStencilOp(DepthStencilOp op)
	{
		if (op == DepthStencilOp::Less)
			g_pd3dDeviceContext->OMSetDepthStencilState(pDSStateLess.Get(), 1);
		else if (op == DepthStencilOp::Less_Equal)
			g_pd3dDeviceContext->OMSetDepthStencilState(pDSStateLessEqual.Get(), 1);
	}
	void RendererBase::BindMainTarget()
	{
		g_pd3dDeviceContext->OMSetRenderTargets(1, g_mainRenderTargetView.GetAddressOf(), pDSV.Get());
	}
}
