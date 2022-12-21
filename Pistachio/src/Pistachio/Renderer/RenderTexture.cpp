#include "ptpch.h"
#include "RenderTexture.h"
#include "DirectX11/DX11RenderTexture.h"
#include "RendererBase.h"
#include "DirectX11/DX11Cubemap.h"
namespace Pistachio {
	void RenderCubeMap::Bind(int slot)
	{
		auto context = RendererBase::Getd3dDeviceContext();
		DX11RenderCubeMap::Bind(&context, renderTargetView[slot], 0);
	}
	void RenderCubeMap::CreateStack(int width, int height, int miplevels)
	{
		renderTargetTexture = DX11Cubemap::Create(width, height, RendererBase::Getd3dDevice(), &shaderResourceView, miplevels);
		DX11RenderCubeMap::Create(shaderResourceView, miplevels, renderTargetView);
	}
	void RenderCubeMap::Clear(float* clearcolor, int slot)
	{
		RendererBase::Getd3dDeviceContext()->ClearRenderTargetView(renderTargetView[slot], clearcolor);
	}
	RenderCubeMap* RenderCubeMap::Create(int width, int height, int miplevels)
	{
		RenderCubeMap* result = new RenderCubeMap;
		DX11RenderCubeMap::Create(result->shaderResourceView, miplevels, result->renderTargetView);
		return result;
	}
	void RenderTexture::Bind()
	{
		auto context = RendererBase::Getd3dDeviceContext();
		DX11RenderCubeMap::Bind(&context, renderTargetView, RendererBase::GetDepthStencilView());
	}
	void RenderTexture::CreateStack(int width, int height, int miplevels)
	{
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.Width = width;
		desc.Height = height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		desc.CPUAccessFlags = 0;
		
		RendererBase::Getd3dDevice()->CreateTexture2D(&desc, NULL, &renderTargetTexture);
		RendererBase::Getd3dDevice()->CreateShaderResourceView(renderTargetTexture, NULL, &shaderResourceView);
		DX11RenderTexture::Create(renderTargetTexture,shaderResourceView, miplevels, &renderTargetView);
		auto device = RendererBase::Getd3dDevice();
		auto context = RendererBase::Getd3dDeviceContext();
	}
	void RenderTexture::Clear(float* clearcolor)
	{
		RendererBase::Getd3dDeviceContext()->ClearRenderTargetView(renderTargetView, clearcolor);
	}
	RenderTexture* RenderTexture::Create(int width, int height, int miplevels)
	{
		return nullptr;
	}
}