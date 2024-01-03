#include "ptpch.h"
#include "../ShadowMap.h"
#include <d3d11_1.h>
#include "../RendererBase.h"
namespace Pistachio
{
	ShadowMap::ShadowMap(const ShadowMap& other)
	{
		PT_PROFILE_FUNCTION();
		Create(other.m_size);
	}
	void ShadowMap::Create(std::uint32_t size)
	{
		PT_PROFILE_FUNCTION();
		m_size = size;
		ID3D11Texture2D* pShadowMap;
		D3D11_TEXTURE2D_DESC texDesc = {};
		texDesc.ArraySize = 1;
		texDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		texDesc.MipLevels = 1;
		texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		texDesc.CPUAccessFlags = 0;
		texDesc.MiscFlags = 0;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.Width = texDesc.Height = size;
		RendererBase::Getd3dDevice()->CreateTexture2D(&texDesc, nullptr, &pShadowMap);

		D3D11_DEPTH_STENCIL_VIEW_DESC dsv = {};
		dsv.Format = DXGI_FORMAT_D32_FLOAT;
		dsv.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		dsv.Texture2D.MipSlice = 0;
		dsv.Texture2D.MipSlice = 0;
		RendererBase::Getd3dDevice()->CreateDepthStencilView(pShadowMap, &dsv, (ID3D11DepthStencilView**)m_DSV.ReleaseAndGetAddressOf());

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Format = DXGI_FORMAT_R32_FLOAT;

		RendererBase::Getd3dDevice()->CreateShaderResourceView(pShadowMap, &srvDesc, (ID3D11ShaderResourceView**)m_SRV.ReleaseAndGetAddressOf());
		RendererBase::Getd3dDeviceContext()->ClearDepthStencilView((ID3D11DepthStencilView*)m_DSV.Get(), D3D11_CLEAR_DEPTH, 0.f, 0);
		pShadowMap->Release();
	}
	void ShadowMap::UpdateSize(std::uint32_t size)
	{
		PT_PROFILE_FUNCTION();
		m_size = size;
		m_DSV = nullptr;
		m_SRV = nullptr;
		Create(size);
	}
	void ShadowMap::Clear(const Region& region)
	{
		PT_PROFILE_FUNCTION();
		float color[4] = { 1.f,1.f,1.f,1.f };
		D3D11_RECT rect;
		rect.left = region.offset.x;
		rect.right = region.offset.x + region.size.x;
		rect.top = region.offset.y;
		rect.bottom = region.offset.y + region.size.y;
		((ID3D11DeviceContext1*)RendererBase::Getd3dDeviceContext())->ClearView((ID3D11DepthStencilView*)m_DSV.Get(), color, &rect, 1);
	}
	void ShadowMap::Bind(int slot)
	{
		PT_PROFILE_FUNCTION();
		RendererBase::Getd3dDeviceContext()->OMSetRenderTargets(slot, nullptr, (ID3D11DepthStencilView*)m_DSV.Get());
	}
	void ShadowMap::BindResource(int slot)
	{
		PT_PROFILE_FUNCTION();
		RendererBase::Getd3dDeviceContext()->PSSetShaderResources(slot, 1, (ID3D11ShaderResourceView* const*)m_SRV.GetAddressOf());
	}
}