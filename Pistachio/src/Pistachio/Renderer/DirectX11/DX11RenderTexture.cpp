#include "ptpch.h"
#include "../RenderTexture.h"
#include "../RendererBase.h" 
namespace Pistachio {
	void RenderCubeMap::Bind(int slot) const
	{
		auto context = RendererBase::Getd3dDeviceContext();
		(context)->OMSetRenderTargets(1, (ID3D11RenderTargetView*const*)m_renderTargetView[slot].GetAddressOf(), 0);
	}
	void RenderCubeMap::BindResource(int slot) const
	{
		RendererBase::Getd3dDeviceContext()->PSSetShaderResources(slot, 1, (ID3D11ShaderResourceView*const*)m_shaderResourceView.GetAddressOf());
	}
	void RenderCubeMap::CreateStack(int width, int height, int miplevels)
	{
		m_width = width;
		m_height = height;
		m_mipLevels = miplevels;
		D3D11_TEXTURE2D_DESC textureDesc;
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
		
		ID3D11Texture2D* m_renderTargetTexture;
		ZeroMemory(&textureDesc, sizeof(textureDesc));

		textureDesc.Width = width;
		textureDesc.Height = height;
		textureDesc.MipLevels = miplevels;
		textureDesc.ArraySize = 6;
		textureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE | D3D11_RESOURCE_MISC_GENERATE_MIPS;

		RendererBase::Getd3dDevice()->CreateTexture2D(&textureDesc, NULL, &m_renderTargetTexture);
		shaderResourceViewDesc.Format = textureDesc.Format;
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
		shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		shaderResourceViewDesc.Texture2D.MipLevels = miplevels;

		RendererBase::Getd3dDevice()->CreateShaderResourceView((m_renderTargetTexture), &shaderResourceViewDesc, (ID3D11ShaderResourceView**)m_shaderResourceView.ReleaseAndGetAddressOf());

		D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
		for (int i = 0; i < 6; i++) {
			renderTargetViewDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
			renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
			renderTargetViewDesc.Texture2DArray.MipSlice = 0;
			renderTargetViewDesc.Texture2DArray.FirstArraySlice = i;
			renderTargetViewDesc.Texture2DArray.ArraySize = 1;
			RendererBase::Getd3dDevice()->CreateRenderTargetView(m_renderTargetTexture, &renderTargetViewDesc, (ID3D11RenderTargetView**)(m_renderTargetView[i].ReleaseAndGetAddressOf()));
		}
		m_renderTargetTexture->Release();
	}
	void RenderCubeMap::Clear(float* clearcolor, int slot)
	{
		RendererBase::Getd3dDeviceContext()->ClearRenderTargetView((ID3D11RenderTargetView*)m_renderTargetView[slot].Get(), clearcolor);
	}
	RenderCubeMap* RenderCubeMap::Create(int width, int height, int miplevels)
	{
		RenderCubeMap* result = new RenderCubeMap;
		result->CreateStack(width, height, miplevels);
		return result;
	}
	RendererID_t RenderCubeMap::GetID()
	{
		RendererID_t ID;
		ID.ptr = m_shaderResourceView.Get();
		return ID;
	}
	Texture2D RenderCubeMap::GetResource()
	{
		Texture2D returnVal;
		returnVal.m_ID = m_shaderResourceView;
		returnVal.m_Height = m_height;
		returnVal.m_Width = m_width;
		returnVal.m_MipLevels = m_mipLevels;
		return returnVal;
	}
	RendererID_t RenderCubeMap::Get_RTID(int slot)
	{
		RendererID_t ID;
		ID.ptr = m_renderTargetView[slot].Get();
		return ID;
	}
	void RenderTexture::CreateStack(const RenderTextureDesc& RTdesc)
	{
		static int i = 0;
		m_width = RTdesc.width;
		m_height = RTdesc.height;
		m_miplevels = RTdesc.miplevels;
		if (m_pDSV);
		m_pDSV = nullptr;
		for (auto format : RTdesc.Attachments.Attachments) {
			bool IsColor = (int)format % 2 == 0;
			ID3D11Texture2D* pTexture;
			D3D11_TEXTURE2D_DESC desc = {};
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
			if (IsColor)
			{
				ID3D11RenderTargetView* pRTV;
				ID3D11ShaderResourceView* pSRV;
				D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
				srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
				srvDesc.Format = desc.Format;
				srvDesc.Texture2D.MipLevels = 1;
				srvDesc.Texture2D.MostDetailedMip = 0;
				RendererBase::Getd3dDevice()->CreateShaderResourceView(pTexture, &srvDesc, &pSRV);
				D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;

				renderTargetViewDesc.Format = desc.Format;
				renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
				renderTargetViewDesc.Texture2D.MipSlice = 0;

				RendererBase::Getd3dDevice()->CreateRenderTargetView(pTexture, &renderTargetViewDesc, &pRTV);
				m_shaderResourceView.push_back(pSRV);
				m_renderTargetView.push_back(pRTV);
				pSRV->Release();
				pRTV->Release();
			}
			else
			{
				D3D11_DEPTH_STENCIL_VIEW_DESC dsv = {};
				dsv.Format = RendererUtils::DXGITextureFormat(format);
				dsv.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
				dsv.Texture2D.MipSlice = 0;
				RendererBase::Getd3dDevice()->CreateDepthStencilView(pTexture, &dsv, (ID3D11DepthStencilView**)(m_pDSV.GetAddressOf()));
			}
			pTexture->Release();
		}

	}
	RenderTexture* RenderTexture::Create(const RenderTextureDesc& rtDesc)
	{
		RenderTexture* result = new RenderTexture;
		result->CreateStack(rtDesc);
		return result;
	}
	void RenderTexture::Bind(int slot, int count) const
	{
		
		if(m_pDSV)
			RendererBase::Getd3dDeviceContext()->OMSetRenderTargets(count, (ID3D11RenderTargetView* const*)m_renderTargetView[slot].GetAddressOf(), ((ID3D11DepthStencilView*)m_pDSV.Get()));
		else
			RendererBase::Getd3dDeviceContext()->OMSetRenderTargets(count, (ID3D11RenderTargetView* const*)m_renderTargetView[slot].GetAddressOf(), nullptr);
		Pistachio::RendererBase::ChangeViewport(m_width, m_height);
	}
	void RenderTexture::BindResource(int slot, int count, int index) const
	{
		RendererBase::Getd3dDeviceContext()->PSSetShaderResources(slot, count, (ID3D11ShaderResourceView*const*)m_shaderResourceView[index].GetAddressOf());
	}
	void RenderTexture::Clear(float* clearcolor, int slot)
	{
		RendererBase::Getd3dDeviceContext()->ClearRenderTargetView((ID3D11RenderTargetView*)m_renderTargetView[slot].Get(), clearcolor);
	}
	void RenderTexture::ClearDepth()
	{
		if (m_pDSV)
			RendererBase::Getd3dDeviceContext()->ClearDepthStencilView((ID3D11DepthStencilView*)m_pDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}
	void RenderTexture::ClearAll(float* clearcolor)
	{
		for (auto& e : m_renderTargetView)
		{
			RendererBase::Getd3dDeviceContext()->ClearRenderTargetView((ID3D11RenderTargetView*)e.Get(), clearcolor);
		}
		if (m_pDSV)
			RendererBase::Getd3dDeviceContext()->ClearDepthStencilView((ID3D11DepthStencilView*)m_pDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}
	RendererID_t RenderTexture::GetSRV(int slot) const { RendererID_t ID; ID.ptr = (void*)m_shaderResourceView[slot].Get(); return ID; }
	Texture2D RenderTexture::GetRenderTexture(int slot) const
	{
		Texture2D returnVal;
		returnVal.m_Height = m_height;
		returnVal.m_Width = m_width;
		returnVal.m_ID = m_shaderResourceView[slot];
		returnVal.m_MipLevels = 1;
		return returnVal;
	}
	RendererID_t RenderTexture::GetRTV(int slot) { RendererID_t ID; ID.ptr = (void*)m_renderTargetView[slot].Get(); return ID; }
	RendererID_t RenderTexture::GetDSV() { RendererID_t ID; ID.ptr = m_pDSV.GetAddressOf(); return ID; }
	Texture2D RenderTexture::GetDepthTexture() const
	{
		Texture2D returnVal;
		returnVal.m_Height = m_height;
		returnVal.m_Width = m_width;
		returnVal.m_ID = m_pDSV;
		returnVal.m_MipLevels = 1;
		return returnVal;
	}
}

