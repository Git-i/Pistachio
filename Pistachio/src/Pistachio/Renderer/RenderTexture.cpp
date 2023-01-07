#include "ptpch.h"
#include "RenderTexture.h"
#include "DirectX11/DX11RenderTexture.h"
#include "DirectX11/DX11Texture.h"
#include "RendererBase.h"
#include "DirectX11/DX11Cubemap.h"
namespace Pistachio {
	DXGI_FORMAT DXGITextureFormat(Pistachio::TextureFormat format) {
		switch (format)
		{
		case Pistachio::TextureFormat::RGBA16F: return DXGI_FORMAT_R16G16B16A16_FLOAT;
			break;
		case Pistachio::TextureFormat::RGBA32F: return DXGI_FORMAT_R32G32B32A32_FLOAT;
			break;
		case Pistachio::TextureFormat::RGBA8U: return DXGI_FORMAT_R8G8B8A8_UNORM;
			break;
		default:
			break;
		}
	}
	void RenderCubeMap::Bind(int slot) const
	{
		auto context = RendererBase::Getd3dDeviceContext();
		DX11RenderCubeMap::Bind(&context, m_renderTargetView[slot], 0);
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
		result->m_width = width;
		result->m_height = height;
		DX11RenderCubeMap::Create(result->m_shaderResourceView, miplevels, result->m_renderTargetView);
		return result;
	}
	RenderTexture::~RenderTexture() {
		Shutdown();
	}
	void RenderTexture::Shutdown() {
		if (m_pDSV) {
			while (m_pDSV->Release()) {};
			m_pDSV = NULL;
		}
		if (m_renderTargetTexture)
		{
			while (m_renderTargetTexture->Release()) {};
			m_renderTargetTexture = NULL;
		}
		if (m_renderTargetView)
		{
			while (m_renderTargetView->Release()) {};
			m_renderTargetView = NULL;
		}
		if (m_shaderResourceView)
		{
			while (m_shaderResourceView->Release()) {};
			m_shaderResourceView = NULL;
		}
	}
	RenderCubeMap::~RenderCubeMap() {};
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
	void RenderTexture::Bind(int slot) const
	{
		auto context = RendererBase::Getd3dDeviceContext();
		DX11RenderCubeMap::Bind(&context, m_renderTargetView, m_pDSV);
	}
	void RenderTexture::BindResource(int slot) const
	{
		DX11Texture::Bind(&m_shaderResourceView, slot);
	}
	void RenderTexture::CreateStack(int width, int height, int miplevels, TextureFormat format)
	{
		m_width = width;
		m_height = height;
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.Width = width;
		desc.Height = height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGITextureFormat(format);
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		desc.CPUAccessFlags = 0;
		
		RendererBase::Getd3dDevice()->CreateTexture2D(&desc, NULL, &m_renderTargetTexture);
		RendererBase::Getd3dDevice()->CreateShaderResourceView(m_renderTargetTexture, NULL, &m_shaderResourceView);
		DX11RenderTexture::Create(m_shaderResourceView, miplevels, &m_renderTargetView);
		auto device = RendererBase::Getd3dDevice();
		auto context = RendererBase::Getd3dDeviceContext();
		DX11RenderTexture::CreateDepth(device, context, &m_pDSV, width, height);
	}
	void RenderTexture::Clear(float* clearcolor)
	{
		RendererBase::Getd3dDeviceContext()->ClearRenderTargetView(m_renderTargetView, clearcolor);
		RendererBase::Getd3dDeviceContext()->ClearDepthStencilView(m_pDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);
	}
	RenderTexture* RenderTexture::Create(int width, int height, int miplevels)
	{
		RenderTexture* result = new RenderTexture;
		result->CreateStack(width, height, miplevels);
		return nullptr;
	}
}