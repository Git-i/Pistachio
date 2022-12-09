#include "ptpch.h"
#include "RendererBase.h"
#include "Pistachio/Core/Log.h"
#include "Pistachio/Core/Window.h"

ID3D11Device* Pistachio::RendererBase::g_pd3dDevice = NULL;
ID3D11DeviceContext* Pistachio::RendererBase::g_pd3dDeviceContext = NULL;
IDXGISwapChain* Pistachio::RendererBase::g_pSwapChain = NULL;
ID3D11RenderTargetView* Pistachio::RendererBase::g_mainRenderTargetView = NULL;
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
			return D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_UNDEFINED;
			break;
		}
	}
	void RendererBase::Shutdown()
	{
	#ifdef PISTACHIO_RENDER_API_DX11
		DX11RendererBase::CleanupRenderTarget(g_mainRenderTargetView);
		DX11RendererBase::CleanupDevice(&g_pSwapChain, &g_pd3dDevice, &g_pd3dDeviceContext);
	#endif
	}
	bool RendererBase::Init(HWND hwnd)
	{
	#ifdef PISTACHIO_RENDER_API_DX11
		g_mainRenderTargetView = DX11RendererBase::CreateDevice(hwnd, &g_pSwapChain, &g_pd3dDevice, &g_pd3dDeviceContext);
		PT_CORE_INFO("RendererBase Initialized with API: DirectX 11");
		RendererBase::Resize((FLOAT)((WindowData*)GetWindowDataPtr())->width, (FLOAT)((WindowData*)GetWindowDataPtr())->height);
		IsDeviceNull = false;
		g_pd3dDeviceContext->IASetPrimitiveTopology(DX11Topology(PrimitiveTopology::TriangleList));
		
		return 0;
	#endif 
	}
	void RendererBase::ClearTarget()
	{
		#ifdef PISTACHIO_RENDER_API_DX11
			DX11RendererBase::CleanupRenderTarget(g_mainRenderTargetView);
		#endif
	}
	void RendererBase::CreateTarget()
	{
		#ifdef PISTACHIO_RENDER_API_DX11
			DX11RendererBase::CreateRenderTarget(g_pSwapChain, g_pd3dDevice, &g_mainRenderTargetView);
		#endif
	}
	void RendererBase::ClearView()
	{
		#ifdef PISTACHIO_RENDER_API_DX11
			RendererBase::g_pd3dDeviceContext->ClearRenderTargetView(RendererBase::g_mainRenderTargetView, m_ClearColor);
		#endif // PISTACHIO_RENDER_API_DX11

	}

	void RendererBase::Resize(float width, float height)
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
		RendererBase::g_pd3dDeviceContext->RSSetViewports(1, &vp);
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

	void RendererBase::DrawIndexed(Buffer& buffer)
	{
		buffer.Bind();
		g_pd3dDeviceContext->DrawIndexed(buffer.ib->GetCount(), 0, 0);
	}


}
