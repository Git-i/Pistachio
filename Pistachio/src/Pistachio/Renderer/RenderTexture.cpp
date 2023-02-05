#include "ptpch.h"
#include "RenderTexture.h"
#include "DirectX11/DX11RenderTexture.h"
#include "DirectX11/DX11Texture.h"
#include "RendererBase.h"
#include "DirectX11/DX11Cubemap.h"
namespace Pistachio {
	
	void RenderCubeMap::Bind(int slot) const
	{
		auto context = RendererBase::Getd3dDeviceContext();
		DX11RenderCubeMap::Bind(&context, &m_renderTargetView[slot], 0);
	}
	void RenderCubeMap::BindResource(int slot) const
	{
		DX11Texture::Bind(&m_shaderResourceView, slot);
	}
	void RenderCubeMap::CreateStack(int width, int height, int miplevels)
	{
		m_width = width;
		m_height = height;
		DX11Cubemap::Create(width, height, RendererBase::Getd3dDevice(), &m_shaderResourceView, &m_renderTargetTexture,miplevels);
		DX11RenderCubeMap::Create(m_shaderResourceView, miplevels, m_renderTargetView);
	}
	void RenderCubeMap::Clear(float* clearcolor, int slot)
	{
		RendererBase::Getd3dDeviceContext()->ClearRenderTargetView(m_renderTargetView[slot], clearcolor);
	}
	RenderCubeMap* RenderCubeMap::Create(int width, int height, int miplevels)
	{
		RenderCubeMap* result = new RenderCubeMap;
		result->CreateStack(width, height, miplevels);
		return result;
	}
	void RenderCubeMap::ShutDown() {
		if (m_pDSV)
		{
			while (m_pDSV->Release()) {};
			m_pDSV = NULL;
		}
		if (m_renderTargetTexture) {
			while(m_renderTargetTexture->Release()){};
			m_renderTargetTexture = NULL;
		}
		if (m_renderTargetView[0]) {
			while (m_renderTargetView[0]->Release()){};
			m_renderTargetView[0] = NULL;
		}
		if (m_renderTargetView[1]) {
			while (m_renderTargetView[1]->Release()){};
			m_renderTargetView[1] = NULL;
		}
		if (m_renderTargetView[2]) {
			while (m_renderTargetView[2]->Release()){};
			m_renderTargetView[2] = NULL;
		}
		if (m_renderTargetView[3]) {
			while (m_renderTargetView[3]->Release()){};
			m_renderTargetView[3] = NULL;
		}
		if (m_renderTargetView[4]) {
			while (m_renderTargetView[4]->Release()){};
			m_renderTargetView[4] = NULL;
		}
		if (m_renderTargetView[5]) {
			while (m_renderTargetView[5]->Release()){};
			m_renderTargetView[5] = NULL;
		}
		if (m_shaderResourceView) {
			while (m_shaderResourceView->Release()){};
			m_shaderResourceView = NULL;
		}
	}
	RenderCubeMap::~RenderCubeMap() {};
	void RenderTexture::CreateStack(const RenderTextureDesc& RTdesc)
	{
		m_width = RTdesc.width;
		m_height = RTdesc.height;
		m_miplevels = RTdesc.miplevels;
		for (auto format : RTdesc.Attachments.Attachments) {
			bool IsColor = (int)format % 2 == 0;
			ID3D11Texture2D* pTexture;
			D3D11_TEXTURE2D_DESC desc;
			ZeroMemory(&desc, sizeof(desc));
			desc.Width = RTdesc.width;
			desc.Height = RTdesc.height;
			desc.MipLevels = 1;
			desc.ArraySize = 1;
			desc.Format = RendererUtils::DXGITextureFormat(format);
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = (IsColor ? D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET : D3D11_BIND_DEPTH_STENCIL);
			desc.CPUAccessFlags = 0;
			RendererBase::Getd3dDevice()->CreateTexture2D(&desc, NULL, &pTexture);
			if ((int)format % 2 == 0)
			{
				ID3D11RenderTargetView* pRTV;
				ID3D11ShaderResourceView* pSRV;
				RendererBase::Getd3dDevice()->CreateShaderResourceView(pTexture, NULL, &pSRV);
				DX11RenderTexture::Create(pSRV, RTdesc.miplevels, &pRTV);
				m_shaderResourceView.push_back(pSRV);
				m_renderTargetView.push_back(pRTV);
			}
			else
			{
				D3D11_DEPTH_STENCIL_VIEW_DESC dsv = {};
				dsv.Format = RendererUtils::DXGITextureFormat(format);
				dsv.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
				dsv.Texture2D.MipSlice = 0;
				RendererBase::Getd3dDevice()->CreateDepthStencilView(pTexture, &dsv, &m_pDSV);
			}
			pTexture->Release();
		}
		
	}
	RenderTexture* RenderTexture::Create(const RenderTextureDesc& rtDesc)
	{
		RenderTexture* result = new RenderTexture;
		result->CreateStack(rtDesc);
		return nullptr;
	}
	void RenderTexture::Bind(int slot, int count) const
	{
		auto context = RendererBase::Getd3dDeviceContext();
		DX11RenderCubeMap::Bind(&context, m_renderTargetView.data(), m_pDSV, count);
		Pistachio::RendererBase::ChangeViewport(m_width, m_height);
	}
	void RenderTexture::BindResource(int slot, int count) const
	{
		DX11Texture::Bind(m_shaderResourceView.data(), slot, count);
	}
	void RenderTexture::Clear(float* clearcolor, int slot)
	{
		RendererBase::Getd3dDeviceContext()->ClearRenderTargetView(m_renderTargetView[slot], clearcolor);
		if(m_pDSV)
		RendererBase::Getd3dDeviceContext()->ClearDepthStencilView(m_pDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);
	}
	void RenderTexture::Resize(int width, int height)
	{
		Shutdown();
		//CreateStack(width, height, m_miplevels, m_format);
	}
	void RenderTexture::Shutdown() {
		if (m_pDSV) {
			while (m_pDSV->Release()) {};
			m_pDSV = NULL;
		}
		for (auto& rtv : m_renderTargetView) {
			if (rtv)
			{
				while (rtv->Release()) {};
				rtv = NULL;
			}
		}
		for (auto& srv : m_shaderResourceView) {
			if (srv)
			{
				while (srv->Release()) {};
				srv = NULL;
			}
		}
		m_shaderResourceView.clear();
		m_renderTargetView.clear();
	}
	RenderTexture::~RenderTexture() {
		Shutdown();
	}
}