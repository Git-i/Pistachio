#include "ptpch.h"
#include "../RenderTexture.h"
#include "../RendererBase.h" 
#define DSV(ID) ((ID3D11DepthStencilView*)ID)
#define DSV_PP(ID) ((ID3D11DepthStencilView**)&ID)

#define RTV_PP(ID) ((ID3D11RenderTargetView**)&ID)
#define RTV_PP_ID_PP(ID) ((ID3D11RenderTargetView**)ID)
#define RTV(ID) ((ID3D11RenderTargetView*)ID)

#define SRV_PP(ID) ((ID3D11ShaderResourceView**)&ID)
#define SRV_PP_ID_PP(ID) ((ID3D11ShaderResourceView**)ID)
#define SRV(ID) ((ID3D11ShaderResourceView*)ID)

#define RESOURCE(ID) ((ID3D11Resource*)ID)
#define RESOURCE_PP(ID) ((ID3D11Resource**)&ID)
#define RESOURCE_PP_ID_PP(ID) ((ID3D11Resource**)ID)
namespace Pistachio {
	void RenderCubeMap::Bind(int slot) const
	{
		auto context = RendererBase::Getd3dDeviceContext();
		(context)->OMSetRenderTargets(1, &m_renderTargetView[slot], 0);
	}
	void RenderCubeMap::BindResource(int slot) const
	{
		RendererBase::Getd3dDeviceContext()->PSSetShaderResources(slot, 1, &m_shaderResourceView);
	}
	void RenderCubeMap::CreateStack(int width, int height, int miplevels)
	{
		m_width = width;
		m_height = height;
		D3D11_TEXTURE2D_DESC textureDesc;
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
		
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

		RendererBase::Getd3dDevice()->CreateShaderResourceView((m_renderTargetTexture), &shaderResourceViewDesc, &m_shaderResourceView);

		D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
		for (int i = 0; i < 6; i++) {
			renderTargetViewDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
			renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
			renderTargetViewDesc.Texture2DArray.MipSlice = 0;
			renderTargetViewDesc.Texture2DArray.FirstArraySlice = i;
			renderTargetViewDesc.Texture2DArray.ArraySize = 1;
			RendererBase::Getd3dDevice()->CreateRenderTargetView(m_renderTargetTexture, &renderTargetViewDesc, &(m_renderTargetView[i]));
		}
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
			while (m_renderTargetTexture->Release()) {};
			m_renderTargetTexture = NULL;
		}
		if (m_renderTargetView[0]) {
			while (m_renderTargetView[0]->Release()) {};
			m_renderTargetView[0] = NULL;
		}
		if (m_renderTargetView[1]) {
			while (m_renderTargetView[1]->Release()) {};
			m_renderTargetView[1] = NULL;
		}
		if (m_renderTargetView[2]) {
			while (m_renderTargetView[2]->Release()) {};
			m_renderTargetView[2] = NULL;
		}
		if (m_renderTargetView[3]) {
			while (m_renderTargetView[3]->Release()) {};
			m_renderTargetView[3] = NULL;
		}
		if (m_renderTargetView[4]) {
			while (m_renderTargetView[4]->Release()) {};
			m_renderTargetView[4] = NULL;
		}
		if (m_renderTargetView[5]) {
			while (m_renderTargetView[5]->Release()) {};
			m_renderTargetView[5] = NULL;
		}
		if (m_shaderResourceView) {
			while (m_shaderResourceView->Release()) {};
			m_shaderResourceView = NULL;
		}
	}
	RenderCubeMap::~RenderCubeMap() {};
	void RenderTexture::CreateStack(const RenderTextureDesc& RTdesc)
	{
		static int i = 0;
		char your_name[8] = "Collins";
		std::cout << your_name << ++i<<std::endl;
		m_width = RTdesc.width;
		m_height = RTdesc.height;
		m_miplevels = RTdesc.miplevels;
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
			}
			else
			{
				D3D11_DEPTH_STENCIL_VIEW_DESC dsv = {};
				dsv.Format = RendererUtils::DXGITextureFormat(format);
				dsv.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
				dsv.Texture2D.MipSlice = 0;
				RendererBase::Getd3dDevice()->CreateDepthStencilView(pTexture, &dsv, DSV_PP(m_pDSV));
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
		auto context = RendererBase::Getd3dDeviceContext();
		(context)->OMSetRenderTargets(count, RTV_PP_ID_PP(&m_renderTargetView[slot]), DSV(m_pDSV));
		Pistachio::RendererBase::ChangeViewport(m_width, m_height);
	}
	void RenderTexture::BindResource(int slot, int count, int index) const
	{
		RendererBase::Getd3dDeviceContext()->PSSetShaderResources(slot, count, SRV_PP_ID_PP(&m_shaderResourceView[index]));
	}
	void RenderTexture::Clear(float* clearcolor, int slot)
	{
		RendererBase::Getd3dDeviceContext()->ClearRenderTargetView(RTV(m_renderTargetView[slot]), clearcolor);
		if (m_pDSV)
			RendererBase::Getd3dDeviceContext()->ClearDepthStencilView(DSV(m_pDSV), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}
	void RenderTexture::Resize(int width, int height)
	{
		Shutdown();
	}
	void RenderTexture::Shutdown() {
		if (m_pDSV) {
			while (DSV(m_pDSV)->Release()) {};
			m_pDSV = NULL;
		}
		for (auto rtv : m_renderTargetView) {
			if (rtv)
			{
				while (RTV(rtv)->Release()) {};
				rtv = NULL;
			}
		}
		for (auto srv : m_shaderResourceView) {
			if (srv)
			{
				while (SRV(srv)->Release()) {};
				srv = NULL;
			}
		}
		m_shaderResourceView.clear();
		m_renderTargetView.clear();
	}
	RenderTexture::~RenderTexture() {
		Shutdown();
	}
	RendererID_t RenderTexture::GetSRV(int slot) const{ return m_shaderResourceView[slot]; }
	RendererID_t RenderTexture::GetRTV(int slot){ return m_renderTargetView[slot]; }
	RendererID_t RenderTexture::GetDSV() { return m_pDSV; }
	void RenderTexture::GetRenderTexture(RendererID_t* pTexture, int slot)const { SRV(m_shaderResourceView[slot])->GetResource(RESOURCE_PP_ID_PP(pTexture)); }
	void RenderTexture::GetDepthTexture(RendererID_t* pTexture) const { return DSV(m_pDSV)->GetResource(RESOURCE_PP_ID_PP(pTexture)); }
}

